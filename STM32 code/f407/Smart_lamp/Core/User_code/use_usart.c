#include "use_usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "control.h"
#include "ui.h"

/**
  * @brief  发送字符串
  * @param  huart: UART句柄
  * @param  str: 要发送的字符串
  * @retval 无
  */
void UES_USART_SendString(UART_HandleTypeDef *huart, const char *str)
{
    HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

// 全局串口DMA处理句柄
static UES_USART_Handle usart_handle;

/**
  * @brief  初始化串口DMA接收功能
  * @param  huart: 指向UART_HandleTypeDef结构的指针，包含串口配置信息
  * @param  hdma: 指向DMA_HandleTypeDef结构的指针，包含DMA配置信息
  * @retval 无
  * @note   此函数会：
  *         1. 使能串口接收中断和空闲中断
  *         2. 使能对应的NVIC中断
  *         3. 清零句柄内存
  *         4. 关联DMA到串口接收
  *         5. 启动DMA接收
  */
void UES_USART_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma)
{
    // 配置DMA为循环模式
    hdma->Init.Mode = DMA_CIRCULAR;
    HAL_DMA_Init(hdma);
	
    huart->Instance->CR1 |= USART_CR1_RXNEIE | USART_CR1_IDLEIE;
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    // 初始化句柄
    memset(&usart_handle, 0, sizeof(usart_handle));
    usart_handle.huart = huart;
    usart_handle.hdma = hdma;
    
    // 关联DMA到串口
    __HAL_LINKDMA(huart, hdmarx, *hdma);
    
    // 启动DMA接收
    UES_USART_Start_DMA_Receive();
}

/**
  * @brief  启动DMA接收
  * @retval 无
  * @note   此函数会：
  *         1. 检查DMA是否已激活，避免重复启动
  *         2. 启动HAL库的DMA接收功能
  *         3. 重置读/写指针位置
  *         4. 设置DMA激活标志
  */
void UES_USART_Start_DMA_Receive(void)
{
    // 检查DMA是否已激活
    if(usart_handle.dma_active) return;
    
    // 启动DMA接收
    HAL_UART_Receive_DMA(usart_handle.huart, 
                         usart_handle.rx_buffer, 
                         UART_RX_BUFFER_SIZE);
    
    usart_handle.dma_active = true;
    usart_handle.rx_read_pos = 0;   // 重置读指针
    usart_handle.rx_write_pos = 0;  // 重置写指针
}

/**
  * @brief  从环形缓冲区读取数据
  * @param  buffer: 指向存储读取数据的缓冲区
  * @param  size: 请求读取的数据大小
  * @retval 实际读取的数据大小
  * @note   此函数会：
  *         1. 检查可用数据量
  *         2. 处理缓冲区回绕情况
  *         3. 更新读指针位置
  *         4. 返回实际读取的数据量
  */
uint16_t UES_USART_Read(uint8_t *buffer, uint16_t size)
{
    uint16_t available = UES_USART_Available();
    if(size == 0 || available == 0) return 0;
    
    size = (size > available) ? available : size;
    uint16_t read_pos = usart_handle.rx_read_pos;
    
    // 单次复制优化
    if(read_pos + size <= UART_RX_BUFFER_SIZE) {
        memcpy(buffer, &usart_handle.rx_buffer[read_pos], size);
        read_pos += size;
    } else {
        // 分段复制
        uint16_t first_part = UART_RX_BUFFER_SIZE - read_pos;
        memcpy(buffer, &usart_handle.rx_buffer[read_pos], first_part);
        memcpy(buffer + first_part, usart_handle.rx_buffer, size - first_part);
        read_pos = size - first_part;
    }
    
    // 更新读指针（确保回绕）
    usart_handle.rx_read_pos = read_pos % UART_RX_BUFFER_SIZE;
    return size;
}

/**
  * @brief  串口空闲中断处理函数
  * @param  huart: 触发中断的串口句柄
  * @retval 无
  * @note   此函数应放在串口中断服务程序(IRQHandler)中调用
  *         主要功能：
  *         1. 检测空闲中断标志
  *         2. 清除空闲标志
  *         3. 计算接收到的数据长度
  *         4. 更新写指针位置
  *         5. 调用接收完成回调函数
  */
void UES_USART_IDLE_IRQHandler(UART_HandleTypeDef *huart)
{
    // 确保中断来自我们管理的串口
    if(huart == usart_handle.huart)
    {
        // 检查空闲中断标志是否置位
        if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
        {
            // 清除空闲中断标志
            __HAL_UART_CLEAR_IDLEFLAG(huart);
            
            // 计算DMA当前传输位置（已接收的数据长度）
            // DMA计数器表示剩余空间，已接收数据 = 总空间 - 剩余空间
            uint16_t dma_pos = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(usart_handle.hdma);
            
            // 更新写指针位置（标记新数据结束位置）
            usart_handle.rx_write_pos = dma_pos % UART_RX_BUFFER_SIZE; // 确保在缓冲区内
            
            // 回调函数通知应用层有新数据到达
            UES_USART_RxCpltCallback();
        }
    }
}

/**
  * @brief  DMA中断处理函数
  * @param  hdma: 触发中断的DMA句柄
  * @retval 无
  * @note   此函数应放在DMA中断服务程序(IRQHandler)中调用
  *         处理两种中断：
  *         1. 传输完成中断（缓冲区满）：重置写指针到缓冲区起始位置
  *         2. 传输错误中断：重启DMA接收
  */
void UES_USART_DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
    // 确保中断来自我们管理的DMA
    if(hdma == usart_handle.hdma)
    {
        // 处理DMA传输完成中断（缓冲区满）
        if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma)))
        {
            // 清除传输完成标志
            __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TC_FLAG_INDEX(hdma));
            
            usart_handle.dma_active = false;
			UES_USART_Start_DMA_Receive();
        }
        
        // 处理DMA传输错误中断
        if(__HAL_DMA_GET_FLAG(hdma, __HAL_DMA_GET_TE_FLAG_INDEX(hdma)))
        {
            // 清除传输错误标志
            __HAL_DMA_CLEAR_FLAG(hdma, __HAL_DMA_GET_TE_FLAG_INDEX(hdma));
            
            // 标记DMA未激活
            usart_handle.dma_active = false;
            
            // 尝试重启DMA接收
            UES_USART_Start_DMA_Receive();
        }
    }
}

/**
  * @brief  获取环形缓冲区中可读数据量
  * @retval 可读数据字节数
  * @note   此函数计算环形缓冲区中未读取的数据量
  *         处理两种情形：
  *         1. 写指针在读指针之后
  *         2. 写指针在读指针之前（缓冲区回绕）
  */
uint16_t UES_USART_Available(void)
{
    // 获取当前读/写位置（防止中断修改）
    uint16_t write_pos = usart_handle.rx_write_pos;
    uint16_t read_pos = usart_handle.rx_read_pos;
    
    if(write_pos >= read_pos) {
        // 写指针在读指针之后（无回绕）
        return write_pos - read_pos;
    } else {
        // 写指针在读指针之前（缓冲区回绕）
        return (UART_RX_BUFFER_SIZE - read_pos) + write_pos;
    }
}

#define CMD_BUFFER_SIZE 128
char command_buffer[CMD_BUFFER_SIZE];
uint8_t cmd_index = 0;
State cmd_state = STATE_WAIT_HEADER;

/**
  * @brief  串口接收完成回调函数
  * @retval 无
  * @note   此函数在空闲中断中被调用，主要功能：
  *         1. 从环形缓冲区读取数据
  *         2. 使用状态机解析数据帧（HEADER开头，FOOTER结尾）
  *         3. 当接收到完整命令帧时调用命令处理函数
  */
void UES_USART_RxCpltCallback(void)
{
    uint8_t rx_buffer[64];
    uint16_t size = UES_USART_Available();
    
    if(size > 0)
    {
        uint16_t read_size = UES_USART_Read(rx_buffer, sizeof(rx_buffer));
        
        // 处理接收到的每个字节
        for(uint16_t i = 0; i < read_size; i++)
        {
            uint8_t c = rx_buffer[i];
            
            switch(cmd_state) 
            {
                case STATE_WAIT_HEADER:
                    if(c == HEADER) 
                    {
                        cmd_state = STATE_RECEIVING;  // 进入处理状态
                        cmd_index = 0;
                    }
                    break;
                    
                case STATE_RECEIVING:
                    if(c == FOOTER)
                    {
                        command_buffer[cmd_index] = '\0';
                        Process_Command(command_buffer);
                        cmd_state = STATE_WAIT_HEADER;
                    } 
                    else if(c == HEADER) 
                    {
                        // 遇到新帧头时重置状态
                        cmd_state = STATE_WAIT_HEADER;
                        cmd_index = 0;
                    }
                    else
                    {
                        if(cmd_index < (CMD_BUFFER_SIZE - 1)) {
                            command_buffer[cmd_index++] = c;
                        } else {
                            // 缓冲区溢出处理
                            cmd_state = STATE_WAIT_HEADER;
                        }
                    }
                    break;
            }
        }
    }
}

/*====================以下是处理接收到完整串口数据包的代码=========================*/

// 定义函数指针类型
typedef void (*CommandHandler)(const char* args);

// 命令结构体
typedef struct {
    const char* cmd_prefix;    // 命令前缀
    CommandHandler handler;    // 处理函数
    uint8_t has_args;          // 是否需要参数
} CommandEntry;

// 命令处理函数声明
void Handle_LED_On(const char* args);
void Handle_LED_Off(const char* args);
void Handle_LED_Breathe(const char* args);
void Handle_LED_Breathe_Stop(const char* args);
void Handle_LED_Flashing_Set(const char* args);
void Handle_LED_Flashing_On(const char* args);
void Handle_LED_Flashing_Off(const char* args);
void Handle_Time_Set(const char* args);
void Handle_Open_Ui_Clock(const char* args);
void Handle_Add_New_Todo(const char* args);
void Handle_Del_Xid_Todo(const char* args);
void Handle_Open_Ui_Todo(const char* args);
void Handle_Clear_Todo_list(const char* args);
void Handle_Open_Eys_Reminder(const char* args);
void Handle_Open_EDrink_Reminder(const char* args);

// 命令表（字典）
static const CommandEntry commandTable[] = {
	// cmd命令
	#if Open_Cmd
    {"LED_ON ",             Handle_LED_On,                 1},  // 1为带参数，0为不带参数
    {"LED_Off",             Handle_LED_Off,                0},
    {"LED_Breathe",         Handle_LED_Breathe,            0},
    {"LED_Breathe_Stop",    Handle_LED_Breathe_Stop,       0},
	{"LED_Flashing_Set ",   Handle_LED_Flashing_Set,      1},
	{"LED_Flashing_On",     Handle_LED_Flashing_On,        0},
	{"LED_Flashing_Off",    Handle_LED_Flashing_Off,       0},
	#endif                  
	// 任务流程指令          
	{"Set_Time ",           Handle_Time_Set,               1},
	{"Open_Ui_Clock",       Handle_Open_Ui_Clock,          0},
	{"Add_New_Todo ",       Handle_Add_New_Todo,           1},
	{"Del_Xid_Todo ",       Handle_Del_Xid_Todo,           1},
	{"Open_Ui_Todo",        Handle_Open_Ui_Todo,           0},
	{"Clear_Todo_list",     Handle_Clear_Todo_list,        0},
	{"Open_Eys_Reminder",   Handle_Open_Eys_Reminder,      0},
	{"Open_Drink_Reminder", Handle_Open_EDrink_Reminder,   0},
};

// 处理完整串口数据包
void Process_Command(const char *cmd) {
	// 处理串口命令
    const size_t tableSize = sizeof(commandTable) / sizeof(CommandEntry);
    
    for(size_t i = 0; i < tableSize; i++) {
        const CommandEntry *entry = &commandTable[i];
        
        // 根据是否需要参数选择匹配方式
        if(entry->has_args ? (strncmp(cmd, entry->cmd_prefix, strlen(entry->cmd_prefix)) == 0)
          : (strcmp(cmd, entry->cmd_prefix) == 0))
        {
            // 提取参数（带参数命令）或传递NULL（无参数命令）
            const char *args = entry->has_args ? cmd + strlen(entry->cmd_prefix) : NULL;
            
            entry->handler(args);
            return;
        }
    }
    
    // 未知命令处理
    char response[64];
    snprintf(response, sizeof(response), "Unknown command: %s\r\n", cmd);
    UES_USART_SendString(&huart1, response);
}

// ===== 具体命令处理实现 =====

/*===============================cmd命令====================================*/
#if Open_Cmd
// cmd命令只是用于开发中调试
void Handle_LED_On(const char* args) {
    if(args == NULL || *args == '\0') {
        UES_USART_SendString(&huart1, "Missing brightness value\r\n");
        return;
    }
    
    char *endptr;
    long brightness = strtol(args, &endptr, 10);
    
    if(endptr == args || brightness > 500) {
        UES_USART_SendString(&huart1, "Invalid brightness value\r\n");
        return;
    }
    
    LED_SetBrightness(brightness);
    LED_On();
    
    char response[32];
    sprintf(response, "LED set to %d\r\n", (uint8_t)brightness);
    UES_USART_SendString(&huart1, response);
}

void Handle_LED_Off(const char* args) {
    (void)args; // 明确标记未使用参数
    LED_Off();
}

void Handle_LED_Breathe(const char* args) {
    (void)args;
    LED_StartBreathe();
}

void Handle_LED_Breathe_Stop(const char* args) {
    (void)args;
    LED_StopBreathe();
}

void Handle_LED_Flashing_Set(const char* args) {
    if (args == NULL || *args == '\0') {
        UES_USART_SendString(&huart1, "Missing parameters. Usage: LED_Flashing_Set <time>,<temp>\r\n");
        return;
    }

    // 复制参数以避免修改原始字符串
    char args_copy[64];
    strncpy(args_copy, args, sizeof(args_copy) - 1);
    args_copy[sizeof(args_copy) - 1] = '\0';

    // 分割参数
    char *time_str = strtok(args_copy, ",");
    char *temp_str = strtok(NULL, ",");

    if (time_str == NULL || temp_str == NULL) {
        UES_USART_SendString(&huart1, "Invalid format. Usage: LED_Flashing_Set <time>,<temp>\r\n");
        return;
    }

    // 验证时间参数
    char *endptr;
    long time_val = strtol(time_str, &endptr, 10);
    if (endptr == time_str || *endptr != '\0') {
        UES_USART_SendString(&huart1, "Invalid time value. Must be integer.\r\n");
        return;
    }
    
    // 验证字符串长度
    size_t temp_len = strlen(temp_str);
    if (temp_len < 8) {
        UES_USART_SendString(&huart1, "Temp string too short. Minimum 8 characters required.\r\n");
        return;
    }

    // 执行实际功能
    LED_Flashing_Set((uint8_t)time_val, temp_str);
    
    // 发送确认响应
    char response[64];
    snprintf(response, sizeof(response), "Flashing set: Time=%d, Temp=%.8s\r\n", 
             (uint8_t)time_val, temp_str);
    UES_USART_SendString(&huart1, response);
}

void Handle_LED_Flashing_On(const char* args)
{
	(void)args;
	LED_Flashing_On();
}

void Handle_LED_Flashing_Off(const char* args)
{
	(void)args;
	LED_Flashing_On();
}
#endif
/*===============================任务流程指令====================================*/

void Handle_Time_Set(const char* args)
{
    // 检查参数是否为空
    if (args == NULL || *args == '\0') {
        UES_USART_SendString(&huart1, "Error: Missing time arguments\r\n");
        return;
    }

    uint16_t hours, minutes, seconds;
    int parsed = sscanf(args, "%hu:%hu:%hu", &hours, &minutes, &seconds);

    // 验证解析结果和数值范围
    if (parsed != 3) {
        UES_USART_SendString(&huart1, "Error: Invalid time format. Use HH:MM:SS\r\n");
        return;
    }

    // 检查时间值有效性
    if (hours > 23 || minutes > 59 || seconds > 59) {
        UES_USART_SendString(&huart1, "Error: Time values out of range\r\n");
        return;
    }

    // 设置时间
    time_clock.Hours = hours;
    time_clock.Minutes = minutes;
    time_clock.Seconds = seconds;
}

void Handle_Open_Ui_Clock(const char* args)
{
	(void)args;
	mode_flag = clock_mode;
	mode_init_flag.clock_mode = 1;
}

// 实现添加待办事项处理函数
void Handle_Add_New_Todo(const char* args)
{
    if (args == NULL || *args == '\0') {
        UES_USART_SendString(&huart1, "Error: Missing arguments. Usage: Add_New_Todo <id>,<task>\r\n");
        return;
    }

    // 复制参数以避免修改原始字符串
    char args_copy[64];
    strncpy(args_copy, args, sizeof(args_copy) - 1);
    args_copy[sizeof(args_copy) - 1] = '\0';

    // 分割参数
    char *id_str = strtok(args_copy, ",");
    char *task_str = strtok(NULL, ",");

    if (id_str == NULL || task_str == NULL) {
        UES_USART_SendString(&huart1, "Error: Invalid format. Usage: Add_New_Todo <id>,<task>\r\n");
        return;
    }

    // 验证ID参数
    char *endptr;
    long id = strtol(id_str, &endptr, 10);
    if (endptr == id_str || *endptr != '\0') {
        UES_USART_SendString(&huart1, "Error: Invalid ID. Must be integer.\r\n");
        return;
    }
    
    // 验证任务长度
    if (strlen(task_str) == 0 || strlen(task_str) > 29) {
        UES_USART_SendString(&huart1, "Error: Task length must be 1-29 characters.\r\n");
        return;
    }

    // 添加待办事项
    Ui_Todo_Add((uint8_t)id, task_str);
    
    // 发送确认响应
    char response[64];
    snprintf(response, sizeof(response), "Added todo: ID=%d, Task=%s\r\n", 
             (uint8_t)id, task_str);
    UES_USART_SendString(&huart1, response);
}

// 实现删除待办事项处理函数
void Handle_Del_Xid_Todo(const char* args)
{
    if (args == NULL || *args == '\0') {
        UES_USART_SendString(&huart1, "Error: Missing ID argument. Usage: Del_Xid_Todo <id>\r\n");
        return;
    }

    // 验证ID参数
    char *endptr;
    long id = strtol(args, &endptr, 10);
    if (endptr == args || *endptr != '\0') {
        UES_USART_SendString(&huart1, "Error: Invalid ID. Must be integer.\r\n");
        return;
    }

    // 删除待办事项
    Ui_Todo_Delete((uint8_t)id);
    
    // 发送确认响应
    char response[32];
    snprintf(response, sizeof(response), "Deleted todo: ID=%d\r\n", (uint8_t)id);
    UES_USART_SendString(&huart1, response);
}

void Handle_Open_Ui_Todo(const char* args)
{
	(void)args;
	mode_flag = todo_mode;
	mode_init_flag.todo_mode = 1;
}

void Handle_Clear_Todo_list(const char* args)
{
	(void)args;
	Ui_Todo_ClearAll();
}

void Handle_Open_Eys_Reminder(const char* args)
{
	(void)args;
	Reminder_time = 5;
	prve_mode = mode_flag;
	mode_flag = Eys_Reminder;
	mode_init_flag.Eys_Reminder = 1;
}
void Handle_Open_EDrink_Reminder(const char* args)
{
	(void)args;
	Reminder_time = 5;
	prve_mode = mode_flag;
	mode_flag = Drink_Reminder;
	mode_init_flag.Drink_Reminder = 1;
}
