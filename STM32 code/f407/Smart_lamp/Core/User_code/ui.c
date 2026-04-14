#include "ui.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

struct time_clock_data time_clock;
struct mode_init_flag_data mode_init_flag;
struct todo_data todo_data_head;
uint8_t time_updated = 0;
uint8_t Reminder_time = 0;
uint16_t bright, tp = 0;
float humidity, temperature;
mode_data mode_flag, prve_mode;

void Ui_Mode_Init_Flag_Init()
{
	mode_init_flag.clock_mode = 1;
	mode_init_flag.todo_mode = 1;
	mode_init_flag.Eys_Reminder = 1;
	mode_init_flag.Drink_Reminder = 1;
}

void Ui_Updata_Init(uint8_t m)
{
	switch(m)
	{
		case (null_mode):
			
		break;
		
		case (clock_mode):
			mode_init_flag.clock_mode = 1;
		break;
		
		case (todo_mode):
			mode_init_flag.todo_mode = 1;
		break;
		
		case (Eys_Reminder):
			mode_init_flag.Eys_Reminder = 1;
		break;
		
		case (Drink_Reminder):
			mode_init_flag.Drink_Reminder = 1;
		break;
	}
}

// 初始化链表
void Ui_Todo_Init()
{
    // 初始化链表头节点
    todo_data_head.id = 0;
    todo_data_head.next_data = &todo_data_head;
    todo_data_head.prev_data = &todo_data_head;
    strcpy(todo_data_head.data, "HEAD NODE");
}

// 添加待办事项
// 添加待办事项
void Ui_Todo_Add(uint8_t id, const char* task)
{
    // 检查ID是否已存在
    struct todo_data* current = todo_data_head.next_data;
    while(current != &todo_data_head) {
        if(current->id == id) {
            // ID已存在，更新任务内容
            strncpy(current->data, task, sizeof(current->data) - 1);
            current->data[sizeof(current->data) - 1] = '\0';
            mode_init_flag.todo_mode = 1; // 设置刷新标志
            return;
        }
        current = current->next_data;
    }
    
    // 创建新节点
    struct todo_data* new_task = (struct todo_data*)malloc(sizeof(struct todo_data));
    
    if(new_task == NULL) {
        return; // 内存分配失败
    }
    
    // 填充数据
    new_task->id = id;
    strncpy(new_task->data, task, sizeof(new_task->data) - 1);
    new_task->data[sizeof(new_task->data) - 1] = '\0'; // 确保字符串终止
    
    // 插入到链表尾部
    struct todo_data* tail = todo_data_head.prev_data;
    
    tail->next_data = new_task;
    new_task->prev_data = tail;
    new_task->next_data = &todo_data_head;
    todo_data_head.prev_data = new_task; // 修复：更新头节点的prev_data指针
    
    mode_init_flag.todo_mode = 1; // 设置刷新标志
}

// 显示待办事项列表
void Ui_Todo_Display()
{
    LCD_Fill(0, 40, 320, 280, BACKGRAND);
    struct todo_data* current = todo_data_head.next_data;
    uint8_t line = 0;
    while(current != &todo_data_head && line < 7) 
	{
        char display_str[40];
        snprintf(display_str, sizeof(display_str), "%d: %s", current->id, current->data);
        LCD_ShowString(10, 55 + line * 40, 300, 12, 12, BACK_COLOR, (uint8_t*)display_str, 1);
        
        current = current->next_data;
        line++;
    }
}

// 删除待办事项
void Ui_Todo_Delete(uint8_t id)
{
    struct todo_data* current = todo_data_head.next_data;
    
    while(current != &todo_data_head) {
        if(current->id == id) 
		{
            struct todo_data* prev = current->prev_data;
            struct todo_data* next = current->next_data;
            
            prev->next_data = next;
            next->prev_data = prev;
            
            free(current); // 释放内存
            return;
        }
        current = current->next_data;
    }
	mode_init_flag.todo_mode = 1;
}

// 清空整个待办事项链表（保留头节点）
void Ui_Todo_ClearAll(void)
{
    struct todo_data* current = todo_data_head.next_data;
    
    // 遍历所有节点（除了头节点）
    while(current != &todo_data_head) {
        struct todo_data* next = current->next_data;
        
        // 从链表中移除当前节点
        current->prev_data->next_data = current->next_data;
        current->next_data->prev_data = current->prev_data;
        
        // 释放内存
        free(current);
        
        // 移动到下一个节点
        current = next;
    }
    
    // 重置头节点指向自身
    todo_data_head.next_data = &todo_data_head;
    todo_data_head.prev_data = &todo_data_head;
    
    // 设置刷新标志
    mode_init_flag.todo_mode = 1;
}

void Ui_Init()
{
	LCD_Clear(BACKGRAND);
	LCD_ShowString(55, 90, 16*25, 16, 16, BLACK, (uint8_t*)"Welcome to the Smart_Lamp", 1);
	Ui_Mode_Init_Flag_Init();
	Ui_Todo_Init();
}

void Ui_ball_check()
{
	static uint16_t x = 115, y = 70, d = 30, g = 3, vy = 0, last_y = 70;
	/*
		x  -> 横坐标
		y  -> 纵坐标
		d  -> 小球半径
		g  -> 重力加速度
		vy -> 垂直分量速度
	*/
	
	vy += g;
	y +=vy;
	
	if (y + 30/2 > 285) {
        y = 285 - 30/2;
        vy = (int)(-vy * 1.0f);
    }
	else if (y - 30/2 < 70) {
        y = 70 + 30/2; 
        vy = (int)(-vy * 0.3f);
    }
	
	if (last_y != y) {
		LCD_Fill(70, last_y - 30, 160, last_y + 30, WHITE);
		last_y = y;
	}
	LCD_Draw_Circle(x, y, d, BLACK);
}

// 绘制表盘和指针
void Ui_Show_Analog_Clock(uint16_t center_x, uint16_t center_y, uint16_t radius) {
    if(time_updated == 1) {
        // 清空表盘区域
        LCD_Fill(center_x - radius, center_y - radius, 
                 center_x + radius, center_y + radius, BACKGRAND);
        
        // 绘制表盘
        LCD_Draw_Circle(center_x, center_y, radius, BACK_COLOR); // 外圆
        LCD_Draw_Circle(center_x, center_y, 5, BACK_COLOR);      // 中心点
        
        // 添加刻度标记（可选）
        for(int i = 0; i < 60; i++) {
            if(i % 5 == 0) {  // 整点位置
                float angle = i * 6 * (PI / 180) - PI/2;  // 转换为弧度
                uint16_t x1 = center_x + (radius * 0.9) * cos(angle);
                uint16_t y1 = center_y + (radius * 0.9) * sin(angle);
                uint16_t x2 = center_x + (radius * 0.95) * cos(angle);
                uint16_t y2 = center_y + (radius * 0.95) * sin(angle);
                LCD_DrawLine(x1, y1, x2, y2, BACK_COLOR);
            } else {  // 分钟位置
                float angle = i * 6 * (PI / 180) - PI/2;
                uint16_t x1 = center_x + (radius * 0.93) * cos(angle);
                uint16_t y1 = center_y + (radius * 0.93) * sin(angle);
                uint16_t x2 = center_x + (radius * 0.95) * cos(angle);
                uint16_t y2 = center_y + (radius * 0.95) * sin(angle);
                LCD_DrawLine(x1, y1, x2, y2, BACK_COLOR);
            }
        }
        
        // 添加数字1-12
        for(int i = 1; i <= 12; i++) {
            float angle = (i * 30 - 90) * (PI / 180);  // 每个数字相隔30度
            uint16_t num_radius = radius * 0.8;         // 数字位置半径
            
            // 计算数字位置
            uint16_t x = center_x + num_radius * cos(angle);
            uint16_t y = center_y + num_radius * sin(angle);
            
            // 调整位置使数字居中
            x -= 6;  // 根据字体大小调整偏移量
            y -= 8;
            
            // 显示数字
            char num_str[3];
            sprintf(num_str, "%d", i);
            LCD_ShowString(x, y, 20, 20, 16, BACK_COLOR, (uint8_t*)num_str, 1);
        }
        
        // 计算指针角度（将时间转换为弧度）
        float hour_angle   = (time_clock.Hours % 12 + time_clock.Minutes / 60.0) * (2 * PI / 12) - PI/2;
        float minute_angle = (time_clock.Minutes) * (2 * PI / 60) - PI/2;
        float second_angle = (time_clock.Seconds) * (2 * PI / 60) - PI/2;
        
        // 计算指针端点坐标
        uint16_t hour_x   = center_x + (radius * 0.5) * cos(hour_angle);
        uint16_t hour_y   = center_y + (radius * 0.5) * sin(hour_angle);
        uint16_t minute_x = center_x + (radius * 0.8) * cos(minute_angle);
        uint16_t minute_y = center_y + (radius * 0.8) * sin(minute_angle);
        uint16_t second_x = center_x + (radius * 0.9) * cos(second_angle);
        uint16_t second_y = center_y + (radius * 0.9) * sin(second_angle);
        
        // 绘制指针（不同颜色和粗细）
        LCD_DrawLine(center_x, center_y, hour_x,   hour_y,   YELLOW);  // 时针
        LCD_DrawLine(center_x, center_y, minute_x, minute_y, GREEN);   // 分针
        LCD_DrawLine(center_x, center_y, second_x, second_y, RED);     // 秒针
    }
    time_updated = 0;
}

void Ui_Null_hand()
{
	return;
}

void Ui_Clock_hand()
{
	if(mode_init_flag.clock_mode)
	{
		LCD_Clear(BACKGRAND);
		mode_init_flag.clock_mode = 0;
		Ui_Show_Analog_Clock(160, 80, 50);
		char temp[30] = "";
		sprintf(temp, "   bright:  %d  ", bright);
		LCD_ShowString(90, 160, 12*11, 12, 12, BACK_COLOR, (uint8_t*)temp, 0);
		sprintf(temp, "   humidity:  %.2f  ", humidity);
		LCD_ShowString(90, 180, 12*11, 12, 12, BACK_COLOR, (uint8_t*)temp, 0);
		sprintf(temp, "   temperature:  %.2f  ", temperature);
		LCD_ShowString(90, 200, 12*11, 12, 12, BACK_COLOR, (uint8_t*)temp, 0);
		
		
	}
	Ui_Clock_run();
}

void Ui_Clock_run()
{
	Ui_Show_Analog_Clock(160, 80, 50);
	char temp[30] = "";
	sprintf(temp, "   bright:  %d  ", bright);
	LCD_ShowString(90, 160, 12*11, 12, 12, BACK_COLOR, (uint8_t*)temp, 0);
	sprintf(temp, "   humidity:  %.2f  ", humidity);
	LCD_ShowString(90, 180, 12*11, 12, 12, BACK_COLOR, (uint8_t*)temp, 0);
	sprintf(temp, "   temperature:  %.2f  ", temperature);
	LCD_ShowString(90, 200, 12*11, 12, 12, BACK_COLOR, (uint8_t*)temp, 0);
}


void Ui_Todo_hand()
{
	if(mode_init_flag.todo_mode)
	{
		LCD_Clear(BACKGRAND);
		LCD_ShowString(80, 10, 16*17, 16, 16, BACK_COLOR, (uint8_t*)"today to do list:", 1);
		Ui_Todo_Display();
		for(int i = 0; i <= 6; i++)
		{
			LCD_DrawLine(0, 40 + i * 40, 320, 40 + i * 40, BACK_COLOR);
		}
		mode_init_flag.todo_mode = 0;
	}
}

void Ui_Eys_Reminder_hand()
{
	if(mode_init_flag.Eys_Reminder)
	{
		LCD_Clear(GRAY);
		LCD_ShowString(50, 70, 16*17, 16, 16, BACK_COLOR, (uint8_t*)"Please drink water on time", 1);
		mode_init_flag.Eys_Reminder = 0;
	}
	if(Reminder_time == 0)
	{
		mode_flag = prve_mode;
		Ui_Updata_Init(prve_mode);
	}
}

void Ui_Drink_Reminder_hand()
{
	if(mode_init_flag.Drink_Reminder)
	{
		LCD_Clear(GRAY);
		LCD_ShowString(50, 70, 16*17, 16, 16, BACK_COLOR, (uint8_t*)"Please rest your eyes", 1);
		mode_init_flag.Drink_Reminder = 0;
	}
	if(Reminder_time == 0)
	{
		mode_flag = prve_mode;
		Ui_Updata_Init(prve_mode);
	}
}

void Ui_Run_Main()
{
	switch(mode_flag)
	{
		case (null_mode):
			Ui_Null_hand();
		break;
		
		case (clock_mode):
			Ui_Clock_hand();
		break;
		
		case (todo_mode):
			Ui_Todo_hand();
		break;
		
		case (Eys_Reminder):
			Ui_Eys_Reminder_hand();
		break;
		
		case (Drink_Reminder):
			Ui_Drink_Reminder_hand();
		break;
	}
}
