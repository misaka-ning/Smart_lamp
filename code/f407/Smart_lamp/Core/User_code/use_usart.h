#ifndef __UES_USART_H
#define __UES_USART_H

#include "stm32f4xx_hal.h"
#include "led.h"
#include "usart.h"
#include <stdbool.h>

#define UART_RX_BUFFER_SIZE 512
#define UART_DMA_TIMEOUT    10
#define HEADER '@'    // 包头标识
#define FOOTER '&'    // 包尾标识

// 命令解析状态
typedef enum {
    STATE_WAIT_HEADER, // 等待包头状态
    STATE_RECEIVING    // 接收数据状态
} State;

// 串口实例结构体
typedef struct {
    UART_HandleTypeDef *huart;
    DMA_HandleTypeDef  *hdma;
    volatile uint16_t rx_read_pos;
    volatile uint16_t rx_write_pos;
    uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
    bool dma_active;
} UES_USART_Handle;

void UES_USART_Init(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
void UES_USART_Start_DMA_Receive(void);
uint16_t UES_USART_Read(uint8_t *buffer, uint16_t size);
uint16_t UES_USART_Available(void);
void UES_USART_IDLE_IRQHandler(UART_HandleTypeDef *huart);
void UES_USART_DMA_IRQHandler(DMA_HandleTypeDef *hdma);
void UES_USART_SendString(UART_HandleTypeDef *huart, const char *str);
void Process_Command(const char *cmd);
void UES_USART_RxCpltCallback(void);

#endif /* __UES_USART_H */
