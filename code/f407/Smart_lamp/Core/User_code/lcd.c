#include "lcd.h"
#include "font.h" 	 
#include "main.h"

//////////////////////////////////////////////////////////////////////////////////	 

/******************************************************************/

//STM32F103ZE核心板
//2.4寸/2.8寸/3.2寸  TFT液晶驱动	  
//支持驱动IC型号包括:ILI9341等	

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


//////////////////////////////////////////////////////////////////////////////////	 


//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

uint32_t  POINT_COLOR;//默认红色    
uint32_t  BACK_COLOR; //背景颜色.默认为白色


/******************************************************************/

//写寄存器函数
//regval:寄存器值

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/
	 

void LCD_WR_REG(uint16_t regval)
{   
	LCD->LCD_REG=regval;//写入要写的寄存器序号	 
}


/******************************************************************/


//写LCD数据
//data:要写入的值

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/

void LCD_WR_DATA(uint16_t data)
{	 
	LCD->LCD_RAM=data;		 
}


/******************************************************************/

//读LCD数据
//返回值:读到的值

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


uint16_t LCD_RD_DATA(void)
{
	volatile uint16_t ram;			//防止被优化
	ram=LCD->LCD_RAM;	
	return ram;	 
}			


/******************************************************************/

//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg;		//写入要写的寄存器序号	 
	LCD->LCD_RAM = LCD_RegValue;//写入数据	    		 
}	  


/******************************************************************/

//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
	HAL_Delay(1);	  
	return LCD_RD_DATA();		//返回读到的值
}  



/******************************************************************/

//开始写GRAM

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/



void LCD_WriteRAM_Prepare(void)
{
 	LCD->LCD_REG=lcddev.wramcmd;	  
}	 



/******************************************************************/

//LCD写GRAM
//RGB_Code:颜色值

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_WriteRAM(uint16_t RGB_Code)
{							    
	LCD->LCD_RAM = RGB_Code;//写十六位GRAM
}



/******************************************************************/

//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


uint16_t LCD_BGR2RGB(uint16_t c)
{
	uint16_t  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
} 



/******************************************************************/

//当mdk -O1时间优化时需要设置
//延时i

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/



void opt_delay(uint8_t i)
{
	while(i--);
}



/******************************************************************/

//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


uint32_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
 	uint16_t r=0,g=0,b=0;
	
	if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回		 
  
	LCD_SetCursor(x,y);	 
	
	if(lcddev.id==0X9341)LCD_WR_REG(0X2E);//发送读GRAM指令
	  
 	r=LCD_RD_DATA();								//dummy Read	   

	opt_delay(2);	  
 	r=LCD_RD_DATA();  		  						//实际坐标颜色

 	if(lcddev.id==0X9341)		//9341要分2次读出
 	{
		opt_delay(2);	  
		b=LCD_RD_DATA(); 
		g=r&0XFF;		//对于9341,第一次读取的是RG的值,R在前,G在后,各占8位
		g<<=8;
	} 
	
  if(lcddev.id==0X9341)return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341需要公式转换一下
	else return LCD_BGR2RGB(r);						//其他IC
}			



/******************************************************************/

//LCD开启显示

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/



void LCD_DisplayOn(void)
{					   
	if(lcddev.id==0X9341)LCD_WR_REG(0X29);	//开启显示
	
}	 

/******************************************************************/

//LCD关闭显示

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_DisplayOff(void)
{	   
	if(lcddev.id==0X9341)LCD_WR_REG(0X28);	//关闭显示
	
}  

/******************************************************************/

//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	 
 	if(lcddev.id==0X9341)
	{		    
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(Xpos>>8);
		LCD_WR_DATA(Xpos&0XFF); 			 
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(Ypos>>8);
		LCD_WR_DATA(Ypos&0XFF); 		
	}
 
} 


/******************************************************************/

//设置LCD的自动扫描方向
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


	   
void LCD_Scan_Dir(uint8_t dir)
{
	uint16_t regval=0;
	
//	uint16_t temp; 
	
	if(lcddev.id==0x9341)
	{
		switch(dir)
		{
			case L2R_U2D://从左到右,从上到下
				regval|=(0<<7)|(0<<6)|(0<<5); 
				break;
			case L2R_D2U://从左到右,从下到上
				regval|=(1<<7)|(0<<6)|(0<<5); 
				break;
			case R2L_U2D://从右到左,从上到下
				regval|=(0<<7)|(1<<6)|(0<<5); 
				break;
			case R2L_D2U://从右到左,从下到上
				regval|=(1<<7)|(1<<6)|(0<<5); 
				break;	 
			case U2D_L2R://从上到下,从左到右
				regval|=(0<<7)|(0<<6)|(1<<5); 
				break;
			case U2D_R2L://从上到下,从右到左
				regval|=(0<<7)|(1<<6)|(1<<5); 
				break;
			case D2U_L2R://从下到上,从左到右
				regval|=(1<<7)|(0<<6)|(1<<5); 
				break;
			case D2U_R2L://从下到上,从右到左
				regval|=(1<<7)|(1<<6)|(1<<5); 
				break;	 
		}
		
		  LCD_WriteReg(0x36,regval|0x08);//改变扫描方向命令  ---此处需要查看数据手册，确定RGB颜色交换位的配置

	//以下设置，为窗口参数设置，设置了全屏的显示范围			
		
			LCD_WR_REG(lcddev.setxcmd); 
			LCD_WR_DATA(0);
		  LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.width-1)>>8);
		  LCD_WR_DATA((lcddev.width-1)&0XFF);
		
			LCD_WR_REG(lcddev.setycmd); 
			LCD_WR_DATA(0);
		  LCD_WR_DATA(0);
			LCD_WR_DATA((lcddev.height-1)>>8);
		  LCD_WR_DATA((lcddev.height-1)&0XFF);  
		
  	}

}

/******************************************************************/

//画点
//x,y:坐标
//POINT_COLOR:此点的颜色

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_SetCursor(x,y);		//设置光标位置 
	LCD_WriteRAM_Prepare();	//开始写入GRAM
	LCD->LCD_RAM=POINT_COLOR; 
}


/******************************************************************/

//快速画点
//x,y:坐标
//color:颜色

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{	   
	if(lcddev.id==0X9341)
	{
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(x>>8);
		LCD_WR_DATA(x&0XFF);  			 
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(y>>8);
		LCD_WR_DATA(y&0XFF); 	
		
		
		LCD->LCD_REG=lcddev.wramcmd; 
		LCD->LCD_RAM=color; 
		
	}
			 

}	


/******************************************************************/

//设置LCD显示方向
//输入参数：

//设置LCD显示方向

////dir:   0,竖屏  正
//         1,竖屏  反
//         2,横屏  左
//         3,横屏  右

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_Display_Dir(uint8_t dir)
{
	uint8_t SCAN_DIR;
		
	if(dir==0)			     //竖屏  正
	{
		lcddev.dir=0;	     //竖屏
		lcddev.width=240;
		lcddev.height=320;
    
		lcddev.wramcmd=0X2C;
	 	lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;

    SCAN_DIR=L2R_U2D; //选择扫描方向		

	}
	
else if (dir==1)			 //横屏
	{	  				
		lcddev.dir=0;	     //竖屏
		lcddev.width=240;
		lcddev.height=320;
    
		lcddev.wramcmd=0X2C;
	 	lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;
		
    SCAN_DIR=R2L_D2U; //选择扫描方向		
	} 	
	
	
	else if (dir==2)			//横屏
	{	  				
		lcddev.dir=1;	     //横屏
		lcddev.width=320;
		lcddev.height=240;

		lcddev.wramcmd=0X2C;
	 	lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;  
		
    SCAN_DIR=U2D_R2L; //选择扫描方向		
		
	} 
 else if (dir==3)				  //横屏
	{	  				
		lcddev.dir=1;	        //横屏
		lcddev.width=320;
		lcddev.height=240;

		lcddev.wramcmd=0X2C;
	 	lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B; 
    
    SCAN_DIR=D2U_L2R; //选择扫描方向				

	} 	
 else //设置默认为竖屏--正
 {
	  lcddev.dir=0;	     //竖屏
		lcddev.width=240;
		lcddev.height=320;
    
		lcddev.wramcmd=0X2C;
	 	lcddev.setxcmd=0X2A;
		lcddev.setycmd=0X2B;

    SCAN_DIR=L2R_U2D; //选择扫描方向		
	 
 }	 


 /////设置屏幕显示--扫描方向
	
	   LCD_Scan_Dir(SCAN_DIR);	//设置屏幕显示--扫描方向
}	 


/******************************************************************/

//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height. 

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{    

	uint16_t twidth,theight;
	
	twidth=sx+width-1;
	theight=sy+height-1;
	
	if(lcddev.id==0X9341)
	{
		LCD_WR_REG(lcddev.setxcmd); 
		LCD_WR_DATA(sx>>8); 
		LCD_WR_DATA(sx&0XFF);	 
		LCD_WR_DATA(twidth>>8); 
		LCD_WR_DATA(twidth&0XFF);  
		LCD_WR_REG(lcddev.setycmd); 
		LCD_WR_DATA(sy>>8); 
		LCD_WR_DATA(sy&0XFF); 
		LCD_WR_DATA(theight>>8); 
		LCD_WR_DATA(theight&0XFF); 
	}

}


/******************************************************************/

//初始化lcd

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_Init(void)
{ 					
	HAL_Delay(50); 					// delay 50 ms 
  
	lcddev.id=LCD_ReadReg(0x0000);	//读ID  
  
	if(lcddev.id<0XFF||lcddev.id==0XFFFF||lcddev.id==0X9300)//读到ID不正确,新增lcddev.id==0X9300判断，因为9341在未被复位的情况下会被读成9300
	{	
 		//尝试9341 ID的读取		
		LCD_WR_REG(0XD3);				   
		lcddev.id=LCD_RD_DATA();	  //dummy read 	
 		lcddev.id=LCD_RD_DATA();	  //读到0X00
  	lcddev.id=LCD_RD_DATA();   	//读取93								   
 		lcddev.id<<=8;
		lcddev.id|=LCD_RD_DATA();  	//读取41 	  
		
 		if(lcddev.id!=0X9341)		//非9341,
		{	
      return;//退出初始化
 		}  	
	}
	
// 	printf(" LCD ID:%x\r\n",lcddev.id); //打印LCD ID  
	
	if(lcddev.id==0X9341)	//9341初始化
	{	 
		LCD_WR_REG(0xCF);  
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0xC1); 
		LCD_WR_DATA(0X30); 
		LCD_WR_REG(0xED);  
		LCD_WR_DATA(0x64); 
		LCD_WR_DATA(0x03); 
		LCD_WR_DATA(0X12); 
		LCD_WR_DATA(0X81); 
		LCD_WR_REG(0xE8);  
		LCD_WR_DATA(0x85); 
		LCD_WR_DATA(0x10); 
		LCD_WR_DATA(0x7A); 
		LCD_WR_REG(0xCB);  
		LCD_WR_DATA(0x39); 
		LCD_WR_DATA(0x2C); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x34); 
		LCD_WR_DATA(0x02); 
		LCD_WR_REG(0xF7);  
		LCD_WR_DATA(0x20); 
		LCD_WR_REG(0xEA);  
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_REG(0xC0);    //Power control 
		LCD_WR_DATA(0x1B);   //VRH[5:0] 
		LCD_WR_REG(0xC1);    //Power control 
		LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0] 
		LCD_WR_REG(0xC5);    //VCM control 
		LCD_WR_DATA(0x30); 	 //3F
		LCD_WR_DATA(0x30); 	 //3C
		LCD_WR_REG(0xC7);    //VCM control2 
		LCD_WR_DATA(0XB7); 
		LCD_WR_REG(0x36);    // Memory Access Control 
		LCD_WR_DATA(0x48); 
		LCD_WR_REG(0x3A);   
		LCD_WR_DATA(0x55); 
		LCD_WR_REG(0xB1);   
		LCD_WR_DATA(0x00);   
		LCD_WR_DATA(0x1A); 
		LCD_WR_REG(0xB6);    // Display Function Control 
		LCD_WR_DATA(0x0A); 
		LCD_WR_DATA(0xA2); 
		LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
		LCD_WR_DATA(0x00); 
		LCD_WR_REG(0x26);    //Gamma curve selected 
		LCD_WR_DATA(0x01); 
		LCD_WR_REG(0xE0);    //Set Gamma 
		LCD_WR_DATA(0x0F); 
		LCD_WR_DATA(0x2A); 
		LCD_WR_DATA(0x28); 
		LCD_WR_DATA(0x08); 
		LCD_WR_DATA(0x0E); 
		LCD_WR_DATA(0x08); 
		LCD_WR_DATA(0x54); 
		LCD_WR_DATA(0XA9); 
		LCD_WR_DATA(0x43); 
		LCD_WR_DATA(0x0A); 
		LCD_WR_DATA(0x0F); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x00); 		 
		LCD_WR_REG(0XE1);    //Set Gamma 
		LCD_WR_DATA(0x00); 
		LCD_WR_DATA(0x15); 
		LCD_WR_DATA(0x17); 
		LCD_WR_DATA(0x07); 
		LCD_WR_DATA(0x11); 
		LCD_WR_DATA(0x06); 
		LCD_WR_DATA(0x2B); 
		LCD_WR_DATA(0x56); 
		LCD_WR_DATA(0x3C); 
		LCD_WR_DATA(0x05); 
		LCD_WR_DATA(0x10); 
		LCD_WR_DATA(0x0F); 
		LCD_WR_DATA(0x3F); 
		LCD_WR_DATA(0x3F); 
		LCD_WR_DATA(0x0F); 
		LCD_WR_REG(0x2B); 
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x01);
		LCD_WR_DATA(0x3f);
		LCD_WR_REG(0x2A); 
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0x00);
		LCD_WR_DATA(0xef);	 
		LCD_WR_REG(0x11); //Exit Sleep
		HAL_Delay(120);
		LCD_WR_REG(0x29); //display on	
	}
	

	LCD_Display_Dir(LCD_DIR_Mode);	//选择--屏幕显示方式
	HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);  //点亮背光
	LCD_Clear(WHITE);
	
	
}  


/******************************************************************/

//清屏函数
//color:要清屏的填充色

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_Clear(uint16_t color)
{
	uint32_t index=0; 
	uint32_t totalpoint;
	
	LCD_Set_Window(0,0,lcddev.width,lcddev.height);//设置全屏窗口
	
  totalpoint=lcddev.width * lcddev.height; 			//得到总点数
	
  LCD_SetCursor(0x00,0x0000);	//设置光标位置 
	LCD_WriteRAM_Prepare();     //开始写入GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD->LCD_RAM=color;	
	}
} 


/******************************************************************/

//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{          
		uint16_t i,j;
		uint16_t xlen=0;

	  xlen=ex-sx+1;	 
		for(i=sy;i<=ey;i++)
		{
		 	LCD_SetCursor(sx,i);      				//设置光标位置 
			LCD_WriteRAM_Prepare();     			//开始写入GRAM	  
			for(j=0;j<xlen;j++)LCD->LCD_RAM=color;	//显示颜色 	    
		}
	
}


/******************************************************************/

//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
	uint16_t height,width;
	uint16_t i,j;
	width=ex-sx+1; 			//得到填充的宽度
	height=ey-sy+1;			//高度
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//设置光标位置 
		LCD_WriteRAM_Prepare();     //开始写入GRAM
		for(j=0;j<width;j++)LCD->LCD_RAM=color[i*width+j];//写入数据 
	}		  
}  

/******************************************************************/

//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
//Color;线条颜色

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t Color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	
	uRow=x1; 
	uCol=y1; 
	
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_Fast_DrawPoint(uRow,uCol,Color);//画点 --使用输入颜色参数 
		
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 


/*******************************************************************************/
//函数：void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
//函数功能：画矩形	  
//输入参数：
//(x1,y1),(x2,y2):矩形的对角坐标
//Color;线条颜色

//桔恩迪电子
//shop58085959.taobao.com

/*******************************************************************************/

void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t Color)
{
	LCD_DrawLine(x1,y1,x2,y1,Color);
	LCD_DrawLine(x1,y1,x1,y2,Color);
	LCD_DrawLine(x1,y2,x2,y2,Color);
	LCD_DrawLine(x2,y1,x2,y2,Color);
}


/*******************************************************************************/
//函数：void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
//函数功能：在指定位置画一个指定大小的圆
//输入参数：
//(x,y):中心点
//r    :半径
//Color;线条颜色

//桔恩迪电子
//shop58085959.taobao.com

/*******************************************************************************/

void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r, uint16_t Color)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_Fast_DrawPoint(x0+a,y0-b,Color);             //5
 		LCD_Fast_DrawPoint(x0+b,y0-a,Color);             //0           
		LCD_Fast_DrawPoint(x0+b,y0+a,Color);             //4               
		LCD_Fast_DrawPoint(x0+a,y0+b,Color);             //6 
		LCD_Fast_DrawPoint(x0-a,y0+b,Color);             //1       
 		LCD_Fast_DrawPoint(x0-b,y0+a,Color);             
		LCD_Fast_DrawPoint(x0-a,y0-b,Color);             //2             
  	LCD_Fast_DrawPoint(x0-b,y0-a,Color);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
}	


/*******************************************************************************/
//函数：LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint16_t color,uint8_t mode)
//函数功能：在指定位置显示一个字符
//输入参数：
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24
//color,字符颜色
//mode:叠加方式(1)还是非叠加方式(0)

//桔恩迪电子
//shop58085959.taobao.com

/*******************************************************************************/

void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint16_t color ,uint8_t mode)
{  							  
  uint8_t temp,t1,t;
	uint16_t y0=y;
	uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数	
	//设置窗口		   
	num=num-' ';//得到偏移后的值
	for(t=0;t<csize;t++)
	{   
		if(size==12)temp=asc2_1206[num][t]; 	 	//调用1206字体
		else if(size==16)temp=asc2_1608[num][t];	//调用1608字体
		else if(size==24)temp=asc2_2412[num][t];	//调用2412字体
		else return;								//没有的字库
		for(t1=0;t1<8;t1++)
		{			    
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,color);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACKGRAND);
			
			temp<<=1;
			y++;
			if(y>=lcddev.height)return;		//超区域了
			if((y-y0)==size)
			{
				y=y0;
				x++;
				if(x>=lcddev.width)return;	//超区域了
				break;
			}
		}  	 
	}  	    	   	 	  
}


/******************************************************************/

//m^n函数
//返回值:m^n次方.

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}	


/*******************************************************************************/
//函数：void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint16_t color)
//函数功能：显示数字,高位为0,则不显示
//输入参数：

//x,y :起点坐标	 
//num:数值(0~4294967295);	
//len :数字的位数
//size:字体大小
//color:颜色 


//返回值:无

//桔恩迪电子
//shop58085959.taobao.com

/*******************************************************************************/

void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint16_t color)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,color,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,color,0); 
	}
} 


/*******************************************************************************/
//函数：void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint16_t color,uint8_t mode)
//函数功能：显示数字,高位为0,还是显示
//输入参数：
//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//color:颜色 
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.

//桔恩迪电子
//shop58085959.taobao.com

/*******************************************************************************/

void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint16_t color,uint8_t mode)
{  
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,color,mode&0X01);  
				else LCD_ShowChar(x+(size/2)*t,y,' ',size,color,mode&0X01);  
 				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,color,mode&0X01); 
	}
} 


/*******************************************************************************/
//函数：void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint16_t color,uint8_t *p)
//函数功能：显示字符串
//输入参数：
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//color:颜色 
//*p:字符串起始地址		  

//桔恩迪电子
//shop58085959.taobao.com

/*******************************************************************************/


void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint16_t color,uint8_t *p, uint8_t flag)
{         
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,color,flag);
        x+=size/2;
        p++;
    }  
}




//****************************************************************************************/
//函数：void Draw_Test(void)
//功能描述: 绘制图形函数测试


//桔恩迪电子
//shop58085959.taobao.com

/****************************************************************************************/

void Draw_Test(void)
{
	
		LCD_Clear(WHITE); //清屏
		
	  LCD_DrawLine(20,64, 220,128,RED);//划线函数
	  LCD_DrawLine(20,128, 220,64,RED);//划线函数
	
	  LCD_DrawRectangle(20,64,220,128, BLUE);//绘制方形状
	  
	  LCD_Draw_Circle(120,96,81, BRED);//绘制圆形
	  LCD_Draw_Circle(120,96,80, BRED);//绘制圆形
	  LCD_Draw_Circle(120,96,79, BRED);//绘制圆形
			
		HAL_Delay(1000);//延时

}


/******************************************************************/

//STM32F103ZE核心板
//2.4寸/2.8寸/3.2寸  TFT液晶驱动	  
//支持驱动IC型号包括:ILI9341等	

//桔恩迪电子
//shop58085959.taobao.com
	

/******************************************************************/


