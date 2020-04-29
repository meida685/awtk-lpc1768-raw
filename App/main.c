/*******************************************************************************
  * Company: Wang Electronic Technology Co., Ltd.
  ******************************************************************************
  * 文件名称：main.c
  * 功能说明：宝马1768_3.2寸LCD test
  * 版    本：V1.1
  * 作    者：openmcu	
  * 日    期: 2014-05-28
********************************************************************************
  * 文件名称：
  * 功能说明：
  * 版    本：
  * 更新作者:	
  * 日    期：
  * 更新原因：
********************************************************************************/                 
#include <LPC17xx.H>                    
#include "lcd.h"
#include "string.h"


extern unsigned char gImage_12[];

/*****************************************************************************
**   Main Function  main()
******************************************************************************/

int main (void)                        
{


  SystemInit();


   LCD_Init();
    LCD_Clear(White);
	  while(1){
	  LCD_Bmp (  0,   0, 240,  320, gImage_12);//显示图片
		}
}

