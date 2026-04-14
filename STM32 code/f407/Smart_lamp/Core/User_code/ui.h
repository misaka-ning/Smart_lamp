#ifndef __UI_H
#define __UI_H

#include "stm32f4xx_hal.h"
#include "lcd.h"

#define PI 3.14

extern uint16_t bright, tp;
extern float humidity, temperature;

struct time_clock_data{
    uint16_t Hours, Minutes, Seconds;
};

struct mode_init_flag_data
{
    uint8_t clock_mode, todo_mode, Eys_Reminder, Drink_Reminder;
};

typedef enum{
    null_mode = 0,
    clock_mode,
    todo_mode,
	Eys_Reminder,
	Drink_Reminder
}mode_data;

struct todo_data
{
    uint8_t id;
    struct todo_data *next_data, *prev_data;  // 修正拼写错误
    char data[30];
};

extern mode_data mode_flag, prve_mode;
extern uint8_t time_updated;
extern uint8_t Reminder_time;
extern struct mode_init_flag_data mode_init_flag;
extern struct time_clock_data time_clock;
extern struct todo_data todo_data_head;  // 声明全局链表头

void Ui_Init(void);
void Ui_ball_check(void);
void Ui_Clock_hand(void);
void Ui_Clock_run(void);
void Ui_Run_Main(void);
void Ui_Eys_Reminder_hand(void);

// 链表操作函数声明
void Ui_Todo_Init(void);
void Ui_Todo_Add(uint8_t id, const char* task);
void Ui_Todo_Delete(uint8_t id);
void Ui_Todo_Display(void);
void Ui_Todo_ClearAll(void);

#endif /* __UI_H */
