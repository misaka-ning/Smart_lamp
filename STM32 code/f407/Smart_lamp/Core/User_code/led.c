#include "led.h"
#include "tim.h"
#include "string.h"

/**
 * @file led.c
 * @brief LED控制模块实现文件（单灯版本）
 * 
 * 本模块通过定时器输出PWM信号，控制LED灯的亮度与实现呼吸灯功能。
 * 支持三种工作模式：关闭状态、正常调光模式、呼吸灯模式。
 * 
 * 硬件要求：
 * - 一个定时器通道配置为PWM输出
 * - 定时器配置参数：
 *   - Prescaler: 168-1
 *   - Counter Period: 500-1
 *   - Auto-reload preload: Enable
 */

// LED状态机定义
typedef enum {
    LED_UNINIT = 0,     // 未初始化状态
    LED_OFF,            // 关闭状态
    LED_ON,             // 正常开启状态
    LED_BREATHE_UP,     // 呼吸灯上升阶段
    LED_BREATHE_DOWN,   // 呼吸灯下降阶段
	LED_FLASHING        // 闪烁状态
} led_state_t;

// LED控制结构体
typedef struct {
    uint8_t bright;         // 亮度 (0-100%)
    led_state_t state;      // 当前LED状态
    
    // 呼吸灯参数
    struct {
        uint16_t current;     // 当前呼吸灯PWM值 (0-500)
        uint32_t last_update; // 最后更新时间戳(ms)
    } breathe;
    
	// 闪烁参数
	struct{
		uint8_t interval_p;                        // 当前指向循环模式的第几位
		uint64_t interval_time;                     // 闪烁间隔时间(单位是ms)
		char interval_flag[LED_FLASHING_MODE_LEN]; // 闪烁循环模式(例如"10110010"即为亮灭亮亮灭灭亮灭...)
		uint32_t last_update;                      // 最后更新时间戳(ms)
	}flashing;
	
    // 保存之前的状态
    struct {
        uint8_t bright;        // 进入呼吸灯或闪烁模式前的亮度
        led_state_t state;     // 进入呼吸灯或闪烁模式前的状态
    } pre_state;       // 呼吸灯或闪烁模式前的状态保存
} led_ctrl_t;

static led_ctrl_t led = {
    .bright = 0,
    .state = LED_UNINIT,
    .breathe = { .current = 500, .last_update = 0 },
	.flashing = {.interval_p = 0, .interval_time = 200, .interval_flag = "10101010", .last_update = 0},
    .pre_state = { .bright = 0, .state = LED_OFF }
};

/* 内部工具函数 --------------------------------------------------*/

/**
 * @brief 将亮度百分比转换为PWM比较值
 * @param brightness 亮度百分比 (0-100)
 * @return PWM比较寄存器值 (500-0)
 * 
 * @note 转换公式: CCR = 500 - (brightness * 5)
 */
static inline uint16_t brightness_to_ccr(uint8_t brightness) {
    return 500 - (brightness * 5);
}

/**
 * @brief 更新PWM输出到硬件
 * 
 * 根据当前设置的亮度和状态更新定时器的比较寄存器值。
 * 仅在正常开启状态(LED_ON)下生效。
 */
static void update_pwm_output(void) {
    if (led.state == LED_ON) {
        __HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, brightness_to_ccr(led.bright));
    }
}

/* 公共API实现 ---------------------------------------------------*/

/**
 * @brief 初始化LED模块
 * 
 * 启动PWM定时器并设置初始状态为关闭。
 * 必须在系统启动时调用一次。
 */
void LED_Init(void) {
    HAL_TIM_PWM_Start(LED_TIM, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, 500);
    led.state = LED_OFF;
}

/**
 * @brief 设置LED亮度
 * @param brightness 亮度值 (0-100)
 * 
 * 设置后立即生效（如果当前处于开启状态）。
 * 当然关闭状态下也可以设置LED亮度，打开时会以最后一次修改的亮度为准。
 * 亮度值超过100时自动限制为100。
 */
void LED_SetBrightness(uint8_t brightness) {
    brightness = (brightness > 100) ? 100 : brightness;
    led.bright = brightness;
    
    // 立即生效
    if (led.state == LED_ON) {
        update_pwm_output();
    }
}

/**
 * @brief 开启LED
 * 
 * 将LED切换到正常开启模式，并应用当前设置的亮度值。
 * 如果已经在开启状态，则无操作。
 */
void LED_On(void) {
    if (led.state == LED_OFF || led.state == LED_UNINIT) {
        led.state = LED_ON;
        update_pwm_output();
    }
}

/**
 * @brief 关闭LED
 * 
 * 关闭LED输出，无论当前处于何种模式。
 * 设置PWM为最大值(500)实现完全关闭。
 */
void LED_Off(void) {
    if (led.state != LED_OFF) {
        __HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, 500);
        led.state = LED_OFF;
    }
}

/**
 * @brief 开启呼吸灯模式
 * 
 * 启动呼吸灯效果，LED同步渐变。
 * 从最高亮度开始逐渐降低。
 * 
 * @note 此函数会保存当前的亮度设置和状态，
 *       以便在停止呼吸灯时恢复。
 */
void LED_StartBreathe(void) {
    if (led.state != LED_BREATHE_UP && led.state != LED_BREATHE_DOWN) {
        led.pre_state.bright = led.bright;
        led.pre_state.state = led.state;
    }
    
    led.state = LED_BREATHE_DOWN;            // 从最高PWM开始下降
    led.breathe.current = 500;               // 初始亮度为0%
    led.breathe.last_update = HAL_GetTick(); // 记录起始时间
}

/**
 * @brief 停止呼吸灯模式
 * 
 * 退出呼吸灯模式并恢复之前的亮度设置和状态。
 * 
 * @note 此函数会恢复到开启呼吸灯之前的状态，
 *       包括亮度和开关状态。
 */
void LED_StopBreathe(void) {
    if (led.state == LED_BREATHE_UP || led.state == LED_BREATHE_DOWN) {
        led.bright = led.pre_state.bright;
        led.state = led.pre_state.state;
        if (led.state == LED_ON) {
            update_pwm_output();
        } else {
            LED_Off();
        }
    }
}

/**
 * @brief 更新呼吸灯状态
 * 
 * 需要在主循环中定期调用(建议每10-20ms调用一次)。
 * 根据状态机更新呼吸灯亮度值并输出到PWM。
 * 非呼吸灯模式下调用此函数无效果。
 */
void LED_Update(void) {
    if (led.state == LED_BREATHE_UP || led.state == LED_BREATHE_DOWN) 
    {
		uint32_t now = HAL_GetTick();
		if (now - led.breathe.last_update < 20) // 约20ms更新一次
			return;
		
		led.breathe.last_update = now; 
		
		if (led.state == LED_BREATHE_DOWN) {
			if (led.breathe.current > 0) {
				led.breathe.current -= 5;
			} else {
				led.state = LED_BREATHE_UP; 
			}
		} else { 
			if (led.breathe.current < 500) {
				led.breathe.current += 5;
			} else {
				led.state = LED_BREATHE_DOWN; 
			}
		}

		__HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, led.breathe.current);
	}
	else if(led.state == LED_FLASHING)
	{
		uint32_t now = HAL_GetTick();
		if (now - led.flashing.last_update < led.flashing.interval_time)
			return;
		
		led.flashing.last_update = now; 
		led.flashing.interval_p = (led.flashing.interval_p + 1) % LED_FLASHING_MODE_LEN;
		if(led.flashing.interval_flag[led.flashing.interval_p] == '1') __HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, 0);
		else __HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, 500);
	}
}

// 设置闪烁参数
void LED_Flashing_Set(uint8_t time, char* temp)
{
	led.flashing.interval_time = time;
	temp[8] = '\0';
	strcpy(led.flashing.interval_flag, temp);
}

// 打开LED灯闪烁
void LED_Flashing_On(void)
{
	if (led.state != LED_FLASHING) {           // 保存之前的状态
        led.pre_state.bright = led.bright;
        led.pre_state.state = led.state;
    }
	led.flashing.interval_p = 0;
	led.flashing.last_update = HAL_GetTick();   // 记录起始时间
	if(led.flashing.interval_flag[led.flashing.interval_p] == '1') __HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, 0);
	else __HAL_TIM_SET_COMPARE(LED_TIM, TIM_CHANNEL_1, 500);
	led.state = LED_FLASHING;
}

// 关闭LED灯闪烁
void LED_Flashing_Off(void)
{
	if (led.state == LED_FLASHING) {
        led.bright = led.pre_state.bright;
        led.state = led.pre_state.state;
        if (led.state == LED_ON) {
            update_pwm_output();
        } else {
            LED_Off();
        }
    }
}
