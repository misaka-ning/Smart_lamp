#ifndef __LED_H
#define __LED_H

#include "main.h"
#include "tim.h"

#define  LED_TIM                  &htim14
#define  LED_FLASHING_MODE_LEN    8

void LED_Init(void);
void LED_SetBrightness(uint8_t brightness);
void LED_On(void);
void LED_Off(void);
void LED_StartBreathe(void);
void LED_StopBreathe(void);
void LED_Update(void);
void LED_Flashing_Set(uint8_t time, char* temp);
void LED_Flashing_On(void);
void LED_Flashing_Off(void);

#endif /* __LED_H */
