/*******************************************************************************
  * Company: Wang Electronic Technology Co., Ltd.
  ******************************************************************************
  * �ļ����ƣ�main.c
  * ����˵��������1768_3.2��LCD test
  * ��    ����V1.1
  * ��    �ߣ�openmcu	
  * ��    ��: 2014-05-28
********************************************************************************
  * �ļ����ƣ�
  * ����˵����
  * ��    ����
  * ��������:	
  * ��    �ڣ�
  * ����ԭ��
********************************************************************************/                 
#include <LPC17xx.H>                    
#include "lcd.h"
#include "string.h"

#include "tkc/platform.h"
#include "lcd/lcd_mem_fragment.h"


void lcd_test(void) {
	rect_t r = rect_init(0, 0, 30, 30);
	lcd_t* lcd = lcd_mem_fragment_create(LCD_W, LCD_H);
	color_t red = color_init(0xff, 0, 0, 0xff);
	color_t green  = color_init(0, 0xff, 0, 0xff);
	color_t blue = color_init(0, 0, 0xff, 0xff);
	
	while(1) {
		lcd_begin_frame(lcd, &r, LCD_DRAW_NORMAL);
		lcd_set_fill_color(lcd, red);
		lcd_fill_rect(lcd, 0, 0, 10, 10);
		lcd_set_fill_color(lcd, green);
		lcd_fill_rect(lcd, 10, 10, 10, 10);
		lcd_set_fill_color(lcd, blue);
		lcd_fill_rect(lcd, 20, 20, 10, 10);
		
		lcd_end_frame(lcd);
	}
}

int main (void)                        
{
  SystemInit();

  LCD_Init();
  LCD_Clear(White);
	
	platform_prepare();
	
	lcd_test();
}

