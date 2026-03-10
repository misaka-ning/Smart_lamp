/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "use_usart.h"
#include "lcd.h"
#include "ui.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint16_t bright_arr[100]        =   {202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 202, 192, 182, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 31, 15, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 15, 31, 46, 62, 77, 92, 106, 120, 133, 146, 159, 171, 182, 192, 202, 211, 219, 226, 226, 226, 226, 212, 212, 212};

// uint16_t bright_arr[100]        =   {208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 209, 209, 209, 209, 209, 209, 209, 209, 209, 207, 207, 207, 207, 207, 207, 209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 207, 207, 207, 207, 207, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208};
// float humidity_arr[100]         =   {26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83, 27.59, 27.59, 27.59, 27.59, 27.59, 27.59, 27.59, 27.59, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.57, 28.57, 28.57, 28.57, 28.57, 28.57, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.7, 28.57, 28.57, 28.57, 28.57, 28.57, 28.57, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 28.18, 27.59, 27.59, 27.59, 27.59, 27.59, 27.59, 27.59, 27.59, 26.83, 26.83, 26.83, 26.83, 26.83, 26.83};
float humidity_arr[100]  = {26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.4, 26.82, 26.82, 25.74, 25.74, 26.58, 26.58, 26.58, 26.58, 26.58, 26.7, 26.7, 25.64, 25.64, 25.64, 25.77, 25.77, 25.77, 25.77, 25.77, 25.77, 25.77, 26.57, 26.57, 26.92, 26.92, 26.92, 26.92, 26.56, 26.56, 26.52, 26.52, 26.4, 26.4, 25.81, 25.81, 25.81, 25.81, 25.49, 25.49, 25.49, 25.49, 25.49, 25.49, 25.26, 25.26, 25.26, 25.34, 25.34, 25.34, 25.34, 25.11, 25.11, 25.11, 26.53, 26.53, 26.53, 26.53, 26.53, 26.53, 26.38, 26.38, 26.38, 26.04, 26.04, 25.72, 25.72, 26.52, 26.52, 26.52, 26.81, 26.81, 26.81, 26.81, 26.81, 25.29, 25.29, 25.29, 25.17, 25.17, 25.89, 25.89, 25.27, 25.27, 25.27, 25.27, 26.03, 26.03, 26.03, 25.53, 25.53, 25.14, 25.14, 25.14};
float temperature_arr[100]      =   {53.92, 53.54, 53.7, 54.41, 54.64, 54.47, 54.63, 53.5, 53.76, 54.61, 53.96, 53.78, 54.25, 54.68, 54.64, 55.0, 54.41, 54.77, 54.71, 53.54, 54.35, 53.87, 53.85, 53.86, 53.65, 54.26, 54.63, 53.84, 53.58, 54.6, 53.85, 53.73, 54.99, 53.76, 54.64, 54.75, 54.25, 54.37, 53.86, 54.91, 53.92, 54.63, 54.84, 53.93, 53.61, 54.93, 54.13, 53.53, 54.79, 54.2, 56.22, 55.92, 56.22, 56.2, 55.82, 56.86, 55.56, 55.56, 56.89, 56.73, 55.65, 56.46, 55.76, 56.05, 55.57, 56.48, 56.57, 56.58, 56.15, 56.67, 56.85, 56.71, 56.4, 56.69, 56.09, 56.82, 56.42, 55.52, 56.75, 55.72, 56.21, 56.81, 56.27, 56.01, 56.77, 55.54, 56.88, 55.87, 56.57, 55.83, 56.14, 56.68, 55.76, 55.73, 55.75, 55.98, 56.13, 56.18, 56.19, 55.98};

	
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM14_Init();
  MX_TIM13_Init();
  MX_USART1_UART_Init();
  MX_FSMC_Init();
  MX_TIM11_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  extern DMA_HandleTypeDef hdma_usart1_rx; 
  UES_USART_Init(&huart1, &hdma_usart1_rx);
  LED_Init();
  LCD_Init();
  Ui_Init();
  HAL_TIM_Base_Init(&htim11);
  HAL_TIM_Base_Init(&htim10);
  MX_TIM11_Init();  // 初始化 TIM11
  MX_TIM10_Init();  // 初始化 TIM10
    
    // 启动 TIM11 中断
    if (HAL_TIM_Base_Start_IT(&htim11) != HAL_OK)
    {
        // 错误处理
        Error_Handler();
    }
	// 启动 TIM10 中断
    if (HAL_TIM_Base_Start_IT(&htim10) != HAL_OK)
    {
        // 错误处理
        Error_Handler();
    }
  UES_USART_SendString(&huart1, "init_finish\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	LED_Update();
	Ui_Run_Main();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // 检查是否是TIM11触发的中断
    if(htim->Instance == TIM11)
    {
        // 秒数增加
        time_clock.Seconds++;
        
        // 检查是否需要进位
        if(time_clock.Seconds >= 60)
        {
            time_clock.Seconds = 0;
            time_clock.Minutes++;
            
            if(time_clock.Minutes >= 60)
            {
                time_clock.Minutes = 0;
                time_clock.Hours++;
                
                if(time_clock.Hours >= 24)
                {
                    time_clock.Hours = 0;
                }
            }
        }     
        time_updated = 1;
		
		if(Reminder_time != 0)
		{
			Reminder_time--;
		}
    }
	
	if(htim->Instance == TIM10)
    {
		bright = bright_arr[tp];
		humidity = humidity_arr[tp];
		temperature = temperature_arr[tp];
		tp = (tp + 2) % 100;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
