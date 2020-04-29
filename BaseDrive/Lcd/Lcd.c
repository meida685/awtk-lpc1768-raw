/*******************************************************************************
  * Company: Wang Electronic Technology Co., Ltd.
  ******************************************************************************
  * �ļ����ƣ�lcd.c
  * ����˵��������17683.2��LCD��������
  * ��    ����V2.1
	* ��    �ߣ�openmcu666
  * ��    �ڣ�2018-11-11
********************************************************************************
  * �ļ����ƣ�
  * ����˵����
  * ��    ����
	* ��������:	
  * ��    �ڣ�
	* ����ԭ��
********************************************************************************/


#include <lpc17xx.h>
#include "LCD.h"
#include "font.h"
#include <stdlib.h>
#include <math.h>
static unsigned short DeviceCode;
/*********************** Hardware specific configuration **********************/

/* 8bit to 16bit LCD Interface 
   
   PINS:
	 -RESET    = P0.17
   - EN      = P0.19
   - LE      = P0.20
   - DIR     = P0.21
   - CS      = P0.22
   - RS      = P0.23 
   - WR      = P0.24 
   - RD      = P0.25
   - DB[0.7] = P2.0...P2.7
   - DB[8.15]= P2.0...P2.7                                                     */


#define DELAY_2N    18

/*---------------------- Graphic LCD size definitions ------------------------*/

#define WIDTH       320                 /* Screen Width (in pixels)           */
#define HEIGHT      240                 /* Screen Hight (in pixels)           */
#define BPP         16                  /* Bits per pixel                     */
#define BYPP        ((BPP+7)/8)         /* Bytes per pixel                    */

#define Font_Size 16                 		/* Ĭ������Ϊ16*8 */
#define XY 1														/* �������л���������1��������0 */


/************************ Local auxiliary functions ***************************/

/*********************************************************************************************************
** ������:    void delay (int cnt)
** ����:       ��ʱ����
** �������:   ��ʱʱ��
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/


 void delay (int cnt) {

  cnt <<= DELAY_2N;
  while (cnt--);
}

__asm void wait()
{
    nop
    BX lr
}

void wait_delay(int count)
{
  while(count--);
}

/*********************************************************************************************************
** ������:     unsigned char lcd_send (unsigned short byte)
** ����:       ��lcd��������
** �������:   Ҫ���͵����ݣ�unsigned short byte
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
static __inline unsigned char lcd_send (unsigned short byte) 
{
  LPC_GPIO2->FIODIR |= 0x000000ff;  //P2.0...P2.7 Output
  LCD_DIR(1)		   				//Interface A->B
  LCD_EN(0)	                        //Enable 2A->2B
  LPC_GPIO2->FIOPIN =  byte;        //Write D0..D7
  LCD_LE(1)
	wait();
	wait();
  LCD_LE(0)							//latch D0..D7
  LPC_GPIO2->FIOPIN =  byte >> 8;   //Write D8..D15 
	return 1;
}

/*********************************************************************************************************
** ������:      unsigned short lcd_read (void)
** ����:        ��lcd��ȡ����
** �������:    ��
** �������:    ��
** ����ֵ:      ��LCD���ص�ֵ
*********************************************************************************************************/
static __inline unsigned short lcd_read (void) 
{
  unsigned short id;
  LPC_GPIO2->FIODIR &= 0xffffff00;                //P2.0...P2.7 Input
  LCD_DIR(0)		   				              //Interface B->A
  LCD_EN(0)	                                      //Enable 2B->2A
  wait_delay(20);							      //delay some times
  id = LPC_GPIO2->FIOPIN & 0x00ff;                //Read D8..D15                         
  LCD_EN(1)	                                      //Enable 1B->1A
  wait_delay(20);							      //delay some times
  id = (id << 8) | (LPC_GPIO2->FIOPIN & 0x00ff);  //Read D0..D7                         
  LCD_DIR(1)
  return(id); 
}

/*********************************************************************************************************
** ������:     void wr_cmd (unsigned char c) 
** ����:       ��lcdд����
** �������:   ���� c
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void wr_cmd (unsigned char c) 
{
  LCD_CS(0)
	LCD_RS(0)
  LCD_RD(1)
	lcd_send(c);  
  LCD_WR(0)
  wait();
  LCD_WR(1)
	LCD_CS(1)
}

/*********************************************************************************************************
** ������:     void wr_dat (unsigned short c) 
** ����:       ��lcdд����
** �������:   ���ݣ�c
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
static __inline void wr_dat (unsigned short c) 
{
  LCD_CS(0)
	LCD_RS(1)
  LCD_RD(1)
  lcd_send(c);
  LCD_WR(0)
  wait();
  LCD_WR(1)
	LCD_CS(1)
}

/*********************************************************************************************************
** ������:     unsigned short rd_dat (void)
** ����:       ��ȡLCD������
** �������:   ��
** �������:   ��
** ����ֵ:     ����LCD������
*********************************************************************************************************/
static __inline unsigned short rd_dat (void) 
{
  unsigned short val = 0;

  LCD_CS(0)
	LCD_RS(1)
  LCD_WR(1)
  LCD_RD(0)
  val = lcd_read();
  LCD_RD(1)
	LCD_CS(1)
  return val;
}

/*********************************************************************************************************
** ������:     void wr_dat_only (unsigned short c)
** ��������:   д������
** �������:   ���� c
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
static __inline void wr_dat_only (unsigned short c) 
{
  LCD_CS(0)
	LCD_RS(1)
	lcd_send(c);
  LCD_WR(0)
  wait();
  LCD_WR(1)
	LCD_CS(1)
}

/*********************************************************************************************************
** ������:     void wr_reg (unsigned char reg, unsigned short val)
** ����:       ���üĴ���
** �������:   �Ĵ�����ַ��unsigned char reg �� ���ݣ�unsigned short val
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
static __inline void wr_reg (unsigned char reg, unsigned short val) 
{
  wr_cmd(reg);
  wr_dat(val);
}

/*********************************************************************************************************
** ������:     unsigned short rd_reg (unsigned short reg) 
** ����:       ���Ĵ�����ֵ
** �������:   �Ĵ�����ַ��unsigned char reg 
** �������:   ��
** ����ֵ:     �Ĵ�����ֵ
*********************************************************************************************************/

static unsigned short rd_reg (unsigned short reg) 
{
  unsigned short val = 0;

  wr_cmd(reg);
  val = rd_dat(); 
  return (val);
}



/*********************************************************************************************************
** ������:      void LCD_Init (void) 
** ����:        LCD��ʼ��
** �������:    ��
** �������:   ��
** ����ֵ:      ��
*********************************************************************************************************/
void LCD_Init (void) 
{ 
  
	
	int i=0;
  LPC_GPIO2->FIODIR   |= 0xFF;				//���ݿ�
											/*	LCD_EN		LCD_LE  	LCD_DIR 	LCD_CS  	LCD_RS  	LCD_WR  	LCD_RD 	LCD_Reset*/
  LPC_GPIO0->FIODIR   |= (1<<19) | (1<<20) | (1<<21) | (1<<22) | (1<<23) | (1<<24) | (1<<25) | (1<<7);		/*P0.19--P0.21           		*/
  LPC_GPIO0->FIOSET   |= (1<<19) | (1<<20) | (1<<21) | (1<<22) | (1<<23) | (1<<24) | (1<<25) | (1<<7);		/*����P0.19--P0.21 Ϊ���		*/
  LPC_GPIO0->FIODIR   |= (1<<8);
	LPC_GPIO0->FIOSET   |= (1<<8);

	LCD_RESET(0);
	delay(10);
	LCD_RESET(1);
	delay(10);

  DeviceCode = rd_reg(0x00);			/* ��ȡ����ID */

	if(DeviceCode==0x9325||DeviceCode==0x9328)			/*ILI9325	  ����9320 С��9325*/
	{
		
	     	wr_reg(0x00e5,0x78F0);      
        wr_reg(0x0001,0x0100);     
        wr_reg(0x0002,0x0700);                   
		wr_reg(0x0003,(1<<12)|(3<<4)|(0<<3) );   
        wr_reg(0x0004,0x0000);                                   
        wr_reg(0x0008,0x0202);	           
        wr_reg(0x0009,0x0000);         
        wr_reg(0x000a,0x0000);         
        wr_reg(0x000c,0x0001);         
        wr_reg(0x000d,0x0000);          
        wr_reg(0x000f,0x0000);
//Power On sequence //
        wr_reg(0x0010,0x0000);   
        wr_reg(0x0011,0x0007);
        wr_reg(0x0012,0x0000);                                                                 
        wr_reg(0x0013,0x0000);
        wr_reg(0x0007,0x0001);                 
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        wr_reg(0x0010,0x1690);   
        wr_reg(0x0011,0x0227);
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        wr_reg(0x0012,0x009d);                  
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        wr_reg(0x0013,0x1900);   
        wr_reg(0x0029,0x0025);
        wr_reg(0x002b,0x000d);
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        wr_reg(0x0020,0x0000);                                                            
        wr_reg(0x0021,0x0000);           
///////////////////////////////////////////////////////      
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        wr_reg(0x0030,0x0007); 
        wr_reg(0x0031,0x0303);   
        wr_reg(0x0032,0x0003);
        wr_reg(0x0035,0x0206);
        wr_reg(0x0036,0x0008); 
        wr_reg(0x0037,0x0406);
        wr_reg(0x0038,0x0304);        
        wr_reg(0x0039,0x0007);     
        wr_reg(0x003c,0x0602);
        wr_reg(0x003d,0x0008);
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        wr_reg(0x0050,0x0000);
        wr_reg(0x0051,0x00ef);                   
        wr_reg(0x0052,0x0000);                   
        wr_reg(0x0053,0x013f); 
        
        wr_reg(0x0060,0xa700);        
        wr_reg(0x0061,0x0001); 
        wr_reg(0x006a,0x0000);
        wr_reg(0x0080,0x0000);
        wr_reg(0x0081,0x0000);
        wr_reg(0x0082,0x0000);
        wr_reg(0x0083,0x0000);
        wr_reg(0x0084,0x0000);
        wr_reg(0x0085,0x0000);
      
        wr_reg(0x0090,0x0010);     
        wr_reg(0x0092,0x0600);  
		   
        wr_reg(0x0007,0x0133);
	}else if(DeviceCode==0x9320||DeviceCode==0x9300)
	{
    /* Start Initial Sequence --------------------------------------------------*/
		wr_reg(0x00, 0x0001);                 /* Start internal OSC                 */
		wr_reg(0x01, 0x0100);                 /* Set SS and SM bit                  */
		wr_reg(0x02, 0x0700);                 /* Set 1 line inversion               */
		wr_reg(0x03, 0x1038);                 /* Set GRAM write direction and BGR=1 */
		wr_reg(0x04, 0x0000);                 /* Resize register                    */
		wr_reg(0x08, 0x0202);                 /* 2 lines each, back and front porch */
		wr_reg(0x09, 0x0000);                 /* Set non-disp area refresh cyc ISC  */
		wr_reg(0x0A, 0x0000);                 /* FMARK function                     */
		wr_reg(0x0C, 0x0000);                 /* RGB interface setting              */
		wr_reg(0x0D, 0x0000);                 /* Frame marker Position              */
		wr_reg(0x0F, 0x0000);                 /* RGB interface polarity             */
		
		/* Power On sequence -------------------------------------------------------*/
		wr_reg(0x10, 0x0000);                 /* Reset Power Control 1              */
		wr_reg(0x11, 0x0000);                 /* Reset Power Control 2              */
		wr_reg(0x12, 0x0000);                 /* Reset Power Control 3              */
		wr_reg(0x13, 0x0000);                 /* Reset Power Control 4              */
		delay(20);                            /* Discharge cap power voltage (200ms)*/
		wr_reg(0x10, 0x17B0);                 /* SAP, BT[3:0], AP, DSTB, SLP, STB   */
		wr_reg(0x11, 0x0137);                 /* DC1[2:0], DC0[2:0], VC[2:0]        */
		delay(5);                             /* Delay 50 ms                        */
		wr_reg(0x12, 0x0139);                 /* VREG1OUT voltage                   */
		delay(5);                             /* Delay 50 ms                        */
		wr_reg(0x13, 0x1D00);                 /* VDV[4:0] for VCOM amplitude        */
		wr_reg(0x29, 0x0013);                 /* VCM[4:0] for VCOMH                 */
		delay(5);                             /* Delay 50 ms                        */
		wr_reg(0x20, 0x0000);                 /* GRAM horizontal Address            */
		wr_reg(0x21, 0x0000);                 /* GRAM Vertical Address              */
		
		/* Adjust the Gamma Curve --------------------------------------------------*/
		wr_reg(0x30, 0x0006);
		wr_reg(0x31, 0x0101);
		wr_reg(0x32, 0x0003);
		wr_reg(0x35, 0x0106);
		wr_reg(0x36, 0x0B02);
		wr_reg(0x37, 0x0302);
		wr_reg(0x38, 0x0707);
		wr_reg(0x39, 0x0007);
		wr_reg(0x3C, 0x0600);
		wr_reg(0x3D, 0x020B);
			
		/* Set GRAM area -----------------------------------------------------------*/
		wr_reg(0x50, 0x0000);                 /* Horizontal GRAM Start Address      */
		wr_reg(0x51, (HEIGHT-1));             /* Horizontal GRAM End   Address      */
		wr_reg(0x52, 0x0000);                 /* Vertical   GRAM Start Address      */
		wr_reg(0x53, (WIDTH-1));              /* Vertical   GRAM End   Address      */
		wr_reg(0x60, 0x2700);                 /* Gate Scan Line                     */
		wr_reg(0x61, 0x0001);                 /* NDL,VLE, REV                       */
		wr_reg(0x6A, 0x0000);                 /* Set scrolling line                 */
		
		/* Partial Display Control -------------------------------------------------*/
		wr_reg(0x80, 0x0000);
		wr_reg(0x81, 0x0000);
		wr_reg(0x82, 0x0000);
		wr_reg(0x83, 0x0000);
		wr_reg(0x84, 0x0000);
		wr_reg(0x85, 0x0000);
		
		/* Panel Control -----------------------------------------------------------*/
		wr_reg(0x90, 0x0010);
		wr_reg(0x92, 0x0000);
		wr_reg(0x93, 0x0003);
		wr_reg(0x95, 0x0110);
		wr_reg(0x97, 0x0000);
		wr_reg(0x98, 0x0000);	   
		wr_reg(0x07, 0x0137);                 /* 262K color and display ON          */ 
	}
	else if(DeviceCode==0x8999)	   //��Ӧ������ICΪSSD1289
	{
		//************* Start Initial Sequence **********//
		wr_reg(0x00, 0x0001); // Start internal OSC.
		wr_reg(0x01, 0x3B3F); // Driver output control, RL=0;REV=1;GD=1;BGR=0;SM=0;TB=1
		wr_reg(0x02, 0x0600); // set 1 line inversion
		//************* Power control setup ************/
		wr_reg(0x0C, 0x0007); // Adjust VCIX2 output voltage
		wr_reg(0x0D, 0x0006); // Set amplitude magnification of VLCD63
		wr_reg(0x0E, 0x3200); // Set alternating amplitude of VCOM
		wr_reg(0x1E, 0x00BB); // Set VcomH voltage
		wr_reg(0x03, 0x6A64); // Step-up factor/cycle setting  7b30
		//************ RAM position control **********/
		wr_reg(0x0F, 0x0000); // Gate scan position start at G0.
		wr_reg(0x44, 0xEF00); // Horizontal RAM address position
		wr_reg(0x45, 0x0000); // Vertical RAM address start position
		wr_reg(0x46, 0x013F); // Vertical RAM address end position
		// ----------- Adjust the Gamma Curve ----------//
		wr_reg(0x30, 0x0000);
		wr_reg(0x31, 0x0706);
		wr_reg(0x32, 0x0206);
		wr_reg(0x33, 0x0300);
		wr_reg(0x34, 0x0002);
		wr_reg(0x35, 0x0000);
		wr_reg(0x36, 0x0707);
		wr_reg(0x37, 0x0200);
		wr_reg(0x3A, 0x0908);
		wr_reg(0x3B, 0x0F0D);
		//************* Special command **************/
		wr_reg(0x28, 0x0006); // Enable test command
		wr_reg(0x2F, 0x12EB); // RAM speed tuning
		wr_reg(0x26, 0x7000); // Internal Bandgap strength
		wr_reg(0x20, 0xB0E3); // Internal Vcom strength
		wr_reg(0x27, 0x0044); // Internal Vcomh/VcomL timing
		wr_reg(0x2E, 0x7E45); // VCOM charge sharing time  
		//************* Turn On display ******************/
		wr_reg(0x10, 0x0000); // Sleep mode off.
		delay(8); // Wait 30mS
		wr_reg(0x11, 0x6870); // Entry mode setup. 262K type B, take care on the data bus with 16it only 
		wr_reg(0x07, 0x0033); // Display ON	*/
	}
	
	  LCD_Clear(Black);
}

/*********************************************************************************************************
** ������:     void GLCD_WindowMax (void) 
** ����:       ���ô������240*320
** �������:   ��
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void GLCD_WindowMax (void) 
{
	if(DeviceCode==0x8999)
	{
		wr_reg(0x44,0|((HEIGHT-1)<<8));
		wr_reg(0x45,0);
		wr_reg(0x46,WIDTH-1);
	}
	else
	{	
		wr_reg(0x50, 0);                      /* Horizontal GRAM Start Address      */
		wr_reg(0x51, HEIGHT-1);               /* Horizontal GRAM End   Address (-1) */
		wr_reg(0x52, 0);                      /* Vertical   GRAM Start Address      */
		wr_reg(0x53, WIDTH-1);                /* Vertical   GRAM End   Address (-1) */
	}
}

/*********************************************************************************************************
** ������:     void LCD_WindowMax (void) 
** ����:        ���ô���
** �������:   x����ʼ�㣺unsigned int x
**											x����  �㣺unsigned int x_end
**											y����ʼ�㣺unsigned int y
**											y����  �㣺unsigned int y_end
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_WindowMax (unsigned int x,unsigned int y,unsigned int x_end,unsigned int y_end) 
{
	if(DeviceCode==0x8999)
	{
		wr_reg(0x44,x|((x_end-1)<<8));
		wr_reg(0x45,y);
		wr_reg(0x46,y_end-1);
	}
	else
	{
		wr_reg(0x50, x);                      /* Horizontal GRAM Start Address      */
		wr_reg(0x51, x_end-1);               /* Horizontal GRAM End   Address (-1) */
		wr_reg(0x52, y);                      /* Vertical   GRAM Start Address      */
		wr_reg(0x53, y_end-1);                /* Vertical   GRAM End   Address (-1) */
	}
}



/*********************************************************************************************************
** ������:     void LCD_SetCursor(uint8_t Xpos, uint16_t Ypos)  
** ����:       ���ù��λ��
** �������:   x�����꣺uint8_t Xpos
**						 y�����꣺uint16_t Ypos
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_SetCursor(uint8_t Xpos, uint16_t Ypos)
{
 	if(DeviceCode==0x8999||DeviceCode==0x9919)
	{
		wr_reg(0x004E, Xpos);
		wr_reg(0X004F, Ypos);
	}
	else 
	{
		wr_reg(0x20, Xpos);
		wr_reg(0x21, Ypos);
	}
}
extern void Delay (uint32_t Time);
/*********************************************************************************************************
** ������:     void LCD_Clear (unsigned short color) 
** ����:        ˢ������ ����ȫ��240X320
** �������:   color ��ɫֵ
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_Clear (unsigned short color) 
{
  unsigned int   i;

  GLCD_WindowMax();
  LCD_SetCursor(0,0);
  wr_cmd(0x22);
  for(i = 0; i < (WIDTH*HEIGHT); i++)
	{
    wr_dat_only(color);
	}
}

/*********************************************************************************************************
** ������:     void LCD_DrawChar64x64(unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
** ����:       ��ʾ�ַ� ��64x64�ַ�
** �������:   x����ʼ�㣺unsigned int x
**						 y����ʼ�㣺unsigned int y
**						 �ַ�ƫ������unsigned char c
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_DrawChar64x64(unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
{	
  uint8_t temp;
  uint16_t pos,t;	
	
		LCD_WindowMax(x,y,x+64,y+64);	   /*���ô���	*/
		LCD_SetCursor(x, y);		
	 
	wr_cmd(0x22);               /*��ʼд��GRAM  */   
	c=c-'0';
	for(pos=0;pos<512;pos++)
	{
		temp=asc2_64x64[c][pos];		 /*����64x64����	*/
		for(t=0;t<8;t++)
	  {                 
	    if(temp&0x01)			   /*�ӵ�λ��ʼ*/
			{
				wr_dat_only(Text_Color);  /*��������ɫ һ����*/
			}
			else
			{
				wr_dat_only(Back_Color);	   /*��������ɫ һ����*/
			}				
	    temp>>=1; 
	  }
	}
	GLCD_WindowMax();;	/*�ָ������С*/	
}
/*********************************************************************************************************
** ������:     void LCD_DrawChar32x32(unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
** ����:       ��ʾ�ַ� ��32x32�ַ�
** �������:   x����ʼ�㣺unsigned int x
**						 y����ʼ�㣺unsigned int y
**						 �ַ�ƫ������unsigned char c
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_DrawChar32x32 (unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
{	
  uint8_t temp;
  uint8_t pos,t;	
	
		LCD_WindowMax(x,y,x+32,y+32);	   /*���ô���	*/
		LCD_SetCursor(x, y);		
	 
	wr_cmd(0x22);               /*��ʼд��GRAM  */   
	c=c-'0';
	for(pos=0;pos<128;pos++)
	{
		temp=asc2_32x32[c][pos];		 /*����32x32����	*/
		for(t=0;t<8;t++)
	  {                 
	    if(temp&0x01)			   /*�ӵ�λ��ʼ*/
			{
				wr_dat_only(Text_Color);  /*��������ɫ һ����*/
			}
			else
			{
				wr_dat_only(Back_Color);	   /*��������ɫ һ����*/
			}				
	    temp>>=1; 
	  }
	}
	GLCD_WindowMax();;	/*�ָ������С*/	
}
/*********************************************************************************************************
** ������:     void LCD_DrawChar16x32(unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
** ����:       ��ʾ�ַ� ��16x32�ַ�
** �������:   x����ʼ�㣺unsigned int x
**						 y����ʼ�㣺unsigned int y
**						 �ַ�ƫ������unsigned char c
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_DrawChar16x32 (unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
{	
  uint8_t temp;
  uint8_t pos,t;	
	
		LCD_WindowMax(x,y,x+16,y+32);	   /*���ô���	*/
		LCD_SetCursor(x, y);		
	 
	wr_cmd(0x22);               /*��ʼд��GRAM  */   
	for(pos=0;pos<64;pos++)
	{
		temp=asc2_16x32[c][pos];		 /*����1608����	*/
		for(t=0;t<8;t++)
	  {                 
	    if(temp&0x01)			   /*�ӵ�λ��ʼ*/
			{
				wr_dat_only(Text_Color);  /*��������ɫ һ����*/
			}
			else
			{
				wr_dat_only(Back_Color);	   /*��������ɫ һ����*/
			}				
	    temp>>=1; 
	  }
	}
	GLCD_WindowMax();;	/*�ָ������С*/	
}
/*********************************************************************************************************
** ������:     void LCD_DrawChar8x16(unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
** ����:       ��ʾ�ַ� ��8x16�ַ�
** �������:   x����ʼ�㣺unsigned int x
**						 y����ʼ�㣺unsigned int y
**						 �ַ�ƫ������unsigned char c
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_DrawChar8x16 (unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
{	
  uint8_t temp;
  uint8_t pos,t;	
	
		LCD_WindowMax(x,y,x+8,y+16);	   /*���ô���	*/
		LCD_SetCursor(x, y);		
	 
	wr_cmd(0x22);               /*��ʼд��GRAM  */   
	for(pos=0;pos<16;pos++)
	{
		temp=asc2_8x16[c][pos];		 /*����1608����	*/
		for(t=0;t<8;t++)
	  {                 
	    if(temp&0x01)			   /*�ӵ�λ��ʼ*/
			{
				wr_dat_only(Text_Color);  /*��������ɫ һ����*/
			}
			else
			{
				wr_dat_only(Back_Color);	   /*��������ɫ һ����*/
			}				
	    temp>>=1; 
	  }
	}
	GLCD_WindowMax();;	/*�ָ������С*/	
}


/*********************************************************************************************************
** ������:     void LCD_DisplayChar(unsigned int x, unsigned int y, unsigned char c,uint16_t Text_Color,uint16_t Back_Color) 
** ����:       ��ʾ�ַ� ��8x16�ַ�
** �������:   x����ʼ�㣺unsigned int x
**						 y����ʼ�㣺unsigned int y
**						 �ַ�ƫ������unsigned char c
**             �ַ���С:size 8 ,16            
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:   ��
** ����ֵ:     ��
*********************************************************************************************************/
void LCD_DisplayChar (unsigned int x, unsigned int y, unsigned char c,uint8_t size,uint16_t Text_Color,uint16_t Back_Color) 
{
      c -= ' ';
	if(size==8)
			LCD_DrawChar8x16(x, y, c,Text_Color,Back_Color);

	else if(size==16){
			LCD_DrawChar16x32(x, y, c,Text_Color,Back_Color);
	}
	
   
}



/*********************************************************************************
** ������:     uint16_t findHzIndex(uint8_t *hz) 
** ����:       �������ִ洢���ڴ��ַ
** �������:   hz ����
**             size �����С 8 /16 /32/64
** �������:   ��
** ����ֵ:     0��ʧ�ܣ� ��=0���غ��ֵĵ�ַ
*********************************************************************************/
uint16_t findHzIndex(uint8_t *hz,uint8_t size)                            /* ���Զ��庺�ֿ��ڲ�����Ҫ��ʾ */
                                                      /* �ĺ��ֵ�λ�� */
{
		uint16_t i=0;
	if(size==8){

	FNT_GB16 *ptGb16 = (FNT_GB16 *)GBHZ_16;		  /*ptGb16ָ��GBHZ_16*/
	while(ptGb16[i].Index[0] > 0x80)
	{
	    if ((*hz == ptGb16[i].Index[0]) && (*(hz+1) == ptGb16[i].Index[1])) /*��������λ����ʾ��ַ��*/
			{
	      return i;
	    }
	    i++;
	    if(i > (sizeof((FNT_GB16 *)GBHZ_16) / sizeof(FNT_GB16) - 1))  /* �����±�Լ�� */
	    {
		    break;
	    }
	}
	
}
   else if(size==16){
		 
	FNT_GB32 *ptGb32 = (FNT_GB32 *)GBHZ_32;		  /*ptGb16ָ��GBHZ_16*/
	while(ptGb32[i].Index[0] > 0x80)
	{
	    if ((*hz == ptGb32[i].Index[0]) && (*(hz+1) == ptGb32[i].Index[1])) /*��������λ����ʾ��ַ��*/
			{
	      return i;
	    }
	    i++;
	    if(i > (sizeof((FNT_GB32 *)GBHZ_32) / sizeof(FNT_GB32) - 1))  /* �����±�Լ�� */
	    {
		    break;
	    }
	}
   }
	 else if(size==32){
		 	FNT_GB32X32 *ptGb32X32 = (FNT_GB32X32 *)GBHZ_32X32;		  /*ptGb16ָ��GBHZ_16*/
	while(ptGb32X32[i].Index[0] > 0x80)
	{
	    if ((*hz == ptGb32X32[i].Index[0]) && (*(hz+1) == ptGb32X32[i].Index[1])) /*��������λ����ʾ��ַ��*/
			{
	      return i;
	    }
	    i++;
	    if(i > (sizeof((FNT_GB32X32 *)GBHZ_32X32) / sizeof(FNT_GB32X32) - 1))  /* �����±�Լ�� */
	    {
		    break;
	    }
	}
	 }
	 
	 else if(size==64){
		 	 	FNT_GB64X64 *ptGb64X64 = (FNT_GB64X64 *)GBHZ_64X64;		  /*ptGb16ָ��GBHZ_16*/
	while(ptGb64X64[i].Index[0] > 0x80)
	{
	    if ((*hz == ptGb64X64[i].Index[0]) && (*(hz+1) == ptGb64X64[i].Index[1])) /*��������λ����ʾ��ַ��*/
			{
	      return i;
	    }
	    i++;
	    if(i > (sizeof((FNT_GB64X64 *)GBHZ_32X32) / sizeof(FNT_GB64X64) - 1))  /* �����±�Լ�� */
	    {
		    break;
	    }
	}
	 }
	return 0;
}

/*********************************************************************************
** ������:     void WriteOneHz(uint16_t x0, uint16_t y0, uint8_t *pucMsk,uint8_t size, uint16_t PenColor, uint16_t BackColor)
** ����:       ��ʾ���֣��˺������ܵ�����Ϊ�����ַ���ʾ	
** �������:   x0,y0     ��ʼ����
**		         *pucMsk   ָ��
               size      �����С 8 /16 /32/64
**				     PenColor	 �ַ���ɫ
**				     BackColor ������ɫ
** �������:   ��
** ����ֵ:   
*********************************************************************************/					
void WriteOneHz(uint16_t x0, uint16_t y0, uint8_t *pucMsk,uint8_t size, uint16_t PenColor, uint16_t BackColor)
{
	
	if(size==8){ //16X16
    uint16_t i,j;
    uint16_t mod[16];                                      /* ��ǰ��ģ 16*16 */
    uint16_t *pusMsk;                                      /* ��ǰ�ֿ��ַ  */
    uint16_t y=0, FONT_Size=16;
     
    pusMsk = (uint16_t *)pucMsk;

    for(i=0; i<16; i++)                                    /* ���浱ǰ���ֵ���ʽ��ģ       */
    {
			mod[i] = *pusMsk;                                /* ȡ�õ�ǰ��ģ�����ֶ������   */
			mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* ��ģ�����ߵ��ֽ�*/
			pusMsk = pusMsk+1;
    }
    y = y0;
		LCD_WindowMax(x0,y0,x0+FONT_Size,y0+FONT_Size);	 	/*���ô���*/
		LCD_SetCursor(x0,y0);                       /*���ù��λ�� */ 
	  wr_cmd(0x22);                /*��ʼд��GRAM*/  
    for(i=0; i<16; i++)                                    /* 16��   */
    {                                              
      for(j=0; j<16; j++)                                /* 16��   */
      {
		    if((mod[i] << j) & 0x8000)       /* ��ʾ��i�� ��16���� */
        { 
			    wr_dat_only(PenColor);
        } 
				else 
				{
          wr_dat_only(BackColor);      /* �ö���ʽ����д�հ׵������*/
				}
      }
        y++;
    }
	LCD_WindowMax(0x0000,0x0000,240,320);  	/*�ָ������С*/
		
	}
	
	else if(size==16){ //16X32
		
		  uint16_t i,j;
    uint16_t mod[32];                                      /* ��ǰ��ģ 16*32 */
    uint16_t *pusMsk;                                      /* ��ǰ�ֿ��ַ  */
    uint16_t y=0, FONT_Size=16;
     
    pusMsk = (uint16_t *)pucMsk;

    for(i=0; i<32; i++)                                    /* ���浱ǰ���ֵ���ʽ��ģ       */
    {
			mod[i] = *pusMsk;                                /* ȡ�õ�ǰ��ģ�����ֶ������   */
			mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* ��ģ�����ߵ��ֽ�*/
			pusMsk = pusMsk+1;
    }
    y = y0;
		LCD_WindowMax(x0,y0,x0+FONT_Size,y0+32);	 	/*���ô���*/
		LCD_SetCursor(x0,y0);                       /*���ù��λ�� */ 
	  wr_cmd(0x22);                /*��ʼд��GRAM*/  
    for(i=0; i<32; i++)                                    /* 32��   */
    {                                              
      for(j=0; j<16; j++)                                /* 16��   */
      {
		    if((mod[i] << j) & 0x8000)       /* ��ʾ��i�� ��16���� */
        { 
			    wr_dat_only(PenColor);
        } 
				else 
				{
          wr_dat_only(BackColor);      /* �ö���ʽ����д�հ׵������*/
				}
      }
        y++;
    }
	LCD_WindowMax(0x0000,0x0000,240,320);  	/*�ָ������С*/
		
		
		
		
	}
  else if(size==32){ //32X32
		  uint16_t i,j;
    uint16_t mod[64];                                      /* ��ǰ��ģ 32*32 */
    uint16_t *pusMsk;                                      /* ��ǰ�ֿ��ַ  */
    uint16_t y=0, FONT_Size=32;
     
    pusMsk = (uint16_t *)pucMsk;

    for(i=0; i<64; i++)                                    /* ���浱ǰ���ֵ���ʽ��ģ       */
    {
			mod[i] = *pusMsk;                                /* ȡ�õ�ǰ��ģ�����ֶ������   */
			mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* ��ģ�����ߵ��ֽ�*/
			pusMsk = pusMsk+1;
    }
    y = y0;
		LCD_WindowMax(x0,y0,x0+FONT_Size,y0+32);	 	/*���ô���*/
		LCD_SetCursor(x0,y0);                       /*���ù��λ�� */ 
	  wr_cmd(0x22);                /*��ʼд��GRAM*/  
    for(i=0; i<64; i++)                                    /* 32��   */
    {                                              
      for(j=0; j<16; j++)                                /* 32��   */
      {
		    if((mod[i] << j) & 0x8000)       /* ��ʾ��i�� ��16���� */
        { 
			    wr_dat_only(PenColor);
        } 
				else 
				{
          wr_dat_only(BackColor);      /* �ö���ʽ����д�հ׵������*/
				}
      }
        y++;
    }
	LCD_WindowMax(0x0000,0x0000,240,320);  	/*�ָ������С*/
		
		
	}

  else if(size==64){ //64X64
		  uint16_t i,j;
    uint16_t mod[256];                                      /* ��ǰ��ģ 32*32 */
    uint16_t *pusMsk;                                      /* ��ǰ�ֿ��ַ  */
    uint16_t y=0, FONT_Size=64;
     
    pusMsk = (uint16_t *)pucMsk;

    for(i=0; i<256; i++)                                    /* ���浱ǰ���ֵ���ʽ��ģ       */
    {
			mod[i] = *pusMsk;                                /* ȡ�õ�ǰ��ģ�����ֶ������   */
			mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* ��ģ�����ߵ��ֽ�*/
			pusMsk = pusMsk+1;
    }
    y = y0;
		LCD_WindowMax(x0,y0,x0+FONT_Size,y0+FONT_Size);	 	/*���ô���*/
		LCD_SetCursor(x0,y0);                       /*���ù��λ�� */ 
	  wr_cmd(0x22);                /*��ʼд��GRAM*/  
    for(i=0; i<256; i++)                                    /* 32��   */
    {                                              
      for(j=0; j<16; j++)                                /* 32��   */
      {
		    if((mod[i] << j) & 0x8000)       /* ��ʾ��i�� ��16���� */
        { 
			    wr_dat_only(PenColor);
        } 
				else 
				{
          wr_dat_only(BackColor);      /* �ö���ʽ����д�հ׵������*/
				}
      }
        y++;
    }
	LCD_WindowMax(0x0000,0x0000,240,320);  	/*�ָ������С*/
		
	}

}



/*********************************************************************************    
** ������:     void LCD_ShowHzString(uint16_t x0, uint16_t y0, uint8_t *pcStr, uint16_t PenColor, uint16_t BackColor)
** ����:       ��ʾ���֣�����������ܵ�������	   
** �������:   x0��y0    ��ʼ����
**		         pcStr     ָ��
**             size      �����С 8 /16 /32/64
**				     PenColor	 �ַ���ɫ
**				     BackColor ������ɫ
** �������:   ��
** ����ֵ:   
*********************************************************************************/
void LCD_ShowHzString(uint16_t x0, uint16_t y0, uint8_t *pcStr, uint8_t size,uint16_t PenColor, uint16_t BackColor)
{
	uint16_t usIndex;
	if(size==8){
	FNT_GB16 *ptGb16 = 0;    
    ptGb16 = (FNT_GB16 *)GBHZ_16; 
  

	usIndex = findHzIndex(pcStr,size);
	WriteOneHz(x0, y0, (uint8_t *)&(ptGb16[usIndex].Msk[0]),size , PenColor, BackColor); /* ��ʾ�ַ� */
		
	}
	
	else if(size==16){
			FNT_GB32 *ptGb32 = 0;    
    ptGb32 = (FNT_GB32 *)GBHZ_32; 
  

	usIndex = findHzIndex(pcStr,size);
	WriteOneHz(x0, y0, (uint8_t *)&(ptGb32[usIndex].Msk[0]),size , PenColor, BackColor); /* ��ʾ�ַ� */
	}
	else if(size==32){
		
		
			FNT_GB32X32 *ptGb32x32 = 0;    
      ptGb32x32 = (FNT_GB32X32 *)GBHZ_32X32; 
  

	usIndex = findHzIndex(pcStr,size);
	WriteOneHz(x0, y0, (uint8_t *)&(ptGb32x32[usIndex].Msk[0]),size , PenColor, BackColor); /* ��ʾ�ַ� */
	}
	
	else if(size==64){
			
			FNT_GB64X64 *ptGb64X64 = 0;    
      ptGb64X64 = (FNT_GB64X64 *)GBHZ_64X64; 
  

	usIndex = findHzIndex(pcStr,size);
	WriteOneHz(x0, y0, (uint8_t *)&(ptGb64X64[usIndex].Msk[0]),size , PenColor, BackColor); /* ��ʾ�ַ� */
	}
	
}
/*********************************************************************************************************
** ������:      void LCD_DisplayAsciiString (unsigned int x, unsigned int y, unsigned char *s,uint8_t size,uint16_t Text_Color,uint16_t Back_Color) 
** ����:       ��ʾASCII�ַ���,�Զ�����,������ʾ���ģ�֧��ASCII�ַ���
** �������:   X��ʼ��unsigned int x
**						 Y��ʼ��unsigned int y
**						 �ַ�����ַ��unsigned char *s
**             �ַ���С:uint8_t size  8��16 (��x��)
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:    ��
** ����ֵ:
*********************************************************************************************************/
void LCD_DisplayAsciiString (unsigned int x, unsigned int y, unsigned char *s,uint8_t size,uint16_t Text_Color,uint16_t Back_Color) 
{
  while (*s) 
	{    
		  
			
			if(*s<0x80){//�ַ�
		LCD_DisplayChar(x,y, *s++,size,Text_Color,Back_Color);
				if(size==16){
					x+=16;
		
				if(x>=223) //һ��240-16-1
			{
				x=0;
				y+=32;
				if(y>=287)y=0;//1�� 320-32-1
			}
					}
				else if(size==8){
						x+=8;
		
				if(x>=231)//һ��240-8-1
			{
				x=0;
				y+=16;
				if(y>=303)
					y=0;//1�� 320-32-1
			}
				}
			}
		
		
		}

 
}

/*********************************************************************************************************
** ������:      void LCD_DisplayHZString (unsigned int x, unsigned int y, unsigned char *s,uint8_t size,uint16_t Text_Color,uint16_t Back_Color) 
** ����:       ��ʾ�����ַ��������ڸú�����ģ��ŵ��ڲ�flash��ռ��оƬ�ռ䣬����ֻ��ȡ��Ҫ�õ��ĺ�����ģ����ģ��Font.h�ļ�
** �������:   X��ʼ��unsigned int x
**						 Y��ʼ��unsigned int y
**						 �ַ�����ַ��unsigned char *s
**             �ַ���С:uint8_t size  8(16x16)  16(16x32) ,32(32x32),64(64x64),
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:    ��
** ����ֵ:
*********************************************************************************************************/
void LCD_DisplayHZString (unsigned int x, unsigned int y, unsigned char *s,uint8_t size,uint16_t Text_Color,uint16_t Back_Color) 
{
  while (*s) 
	{    
		  
			
			if(*s>0x80){//����
			//�Զ��庺����ʾ
			LCD_ShowHzString(x, y, s,size, Text_Color, Back_Color);
				
				if(size==8){ //16X16����
					
				  s += 2;
		   x+=16;
		
				if(x>=231) //һ��240-16-1
			{
				x=0;
				y+=16;
				if(y>=320)//һ�г���
					y=0;
			}	
					
				}
				else if(size==16){//16X32����
					
			  s += 2;
		   x+=16;
		
				if(x>=231) //һ��240-16-1
			{
				x=0;
				y+=32;
				if(y>=303)
					y=0;//1�� 320-16-1
			}
			
		}
				else if(size==32){//32X32����
						s += 2;
		   x+=32;
		
				if(x>=224) //һ��240-8-1
			{
				x=0;
				y+=32;
				if(y>=303)
					y=0;//1�� 320-16-1
			}
				}
				
		else if(size==64){//64X64����
			
			
						s += 2;
		   x+=64;
		
				if(x>=191) //һ��240-64-1
			{
				x=0;
				y+=64;
				if(y>=287)
					y=0;//1�� 320-16-1
			}
		}
			}
			
		
		}

 
}
/*********************************************************************************************************
** ������:      void LCD_DisplayStringCust(uint8_t x,uint8_t y,char *c,uint16_t Text_Color,uint16_t Back_Color)
** ����:       �Զ���ASCII�ַ�����ʾ,�Զ����У�������ʾ���ģ�����Ascii��ģ��Ҫ�Լ���ȡ����ģ��Font.h�ļ�
** �������:   X��ʼ��unsigned int x
**						 Y��ʼ��unsigned int y
**						 �ַ�����ַ��unsigned char *s
**             �ַ���С:uint8_t size  32��64(��
**             ������ɫ:Text_Color
**             ������ɫ:Back_Color
** �������:    ��
** ����ֵ:
*********************************************************************************************************/
void LCD_DisplayAsciiStringCust(uint8_t x,uint16_t y,char *c,uint8_t size,uint16_t Text_Color,uint16_t Back_Color){

	
	while(*c){
		
	if(size==32){
		LCD_DrawChar32x32(x, y,*c++,Text_Color,Back_Color);//��ʾ32x32�ַ�
		
		x+=32;
		if(x>=224){//
			y+=32;
			x=0;
			if(y>=303){//
				  y=0;
			}
			
			}
		}
	
		else if(size==64){
			
			LCD_DrawChar64x64(x, y,*c++,Text_Color,Back_Color);//��ʾ64x64�ַ�
			
				x+=64;
		if(x>=175){//240-64-1
			y+=64;
			x=0;
			if(y>=255){//320-64-1
				  y=0;
			}
			
			}
		}
		
	}
		
	}
	

/*********************************************************************************************************
** ������:              void LCD_Bmp (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bmp)  
** ����:                ��ʾBMPͼƬ
** �������:            x����ʼ�㣺unsigned int x
**											y����ʼ�㣺unsigned int y
**											ͼƬ��ȣ�unsigned int w
**											ͼƬ�߶ȣ�unsigned int h
**											ͼƬ�����ַ��unsigned char *bmp
** �������:    ��
** ����ֵ:
*********************************************************************************************************/
void LCD_Bmp (unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bmp) 
{
  unsigned int    i, j;
  unsigned short *bitmap_ptr = (unsigned short *)bmp;

  LCD_WindowMax(x,y,x+w,y+h);

  LCD_SetCursor(x,y);
  wr_cmd(0x22);
  
 bitmap_ptr += (h*w)-1;
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      wr_dat_only(*bitmap_ptr--);
	
    }
  }
}

/*********************************************************************************
** ������:              void LCD_DrawPoint(uint16_t xsta, uint16_t ysta,uint16_t POINT_COLOR) 
** ����:                ��㺯��
** �������:            xsta X��ʼ���� 0~239
**										  ysta	Y��ʼ���� 0~319
**											POINT_COLORָ�������ɫ
** �������:    ��
** ����ֵ:
*********************************************************************************/
void LCD_DrawPoint(uint16_t xsta, uint16_t ysta,uint16_t POINT_COLOR)
{

	LCD_WindowMax (xsta,ysta, 240, 320);
	LCD_SetCursor(xsta,ysta);  /*���ù��λ��  */

	wr_cmd(0x22);           /*��ʼд��GRAM */

	wr_dat_only(POINT_COLOR);

	/*�ָ�����*/
	LCD_WindowMax (0, 0, 240, 320); 
}

/*********************************************************************************************************
** ������:              void Cube(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color)
** ����:                ��ʾ������
** �������:            x����ʼ�㣺unsigned int x
**											y����ʼ�㣺unsigned int y
**											��ȣ�     unsigned int w
**										  �߶ȣ�     unsigned int h
** �������:            ��
** ����ֵ:              ��
*********************************************************************************************************/
void Cube(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color)
{
  
	  uint16_t j,i;

  LCD_WindowMax(x,y,x+w,y+h);

  LCD_SetCursor(x,y);
  wr_cmd(0x22);
  
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      wr_dat_only(color);
    }
  }
}

/*********************************************************************************
** ������:              void LCD_DrawLine(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend)
** ����:                ָ������(����)������
** �������:            xsta X��ʼ����
**										  ysta Y��ʼ����
**										  xend X��ʼ����
**										  yend Y�յ�����	
** �������:            ��
** ����ֵ:              ��
*********************************************************************************/ 
void LCD_DrawLine(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t POINT_COLOR)
{
    uint16_t x, y, t;
	if((xsta==xend)&&(ysta==yend)){
			LCD_DrawPoint(xsta, ysta,POINT_COLOR);
	}
	
	else if(abs(yend-ysta)>abs(xend-xsta))/*б�ʴ���1 */ 
	{
		if(ysta>yend) 
		{
			t=ysta;
			ysta=yend;
			yend=t; 
			t=xsta;
			xsta=xend;
			xend=t; 
		}
		for(y=ysta;y<yend;y++)            /*��y��Ϊ��׼*/ 
		{
			x=(uint32_t)(y-ysta)*(xend-xsta)/(yend-ysta)+xsta;
			LCD_DrawPoint(x, y,POINT_COLOR);  
		}
	}
	else     /*б��С�ڵ���1 */
	{
		if(xsta>xend)
		{
			t=ysta;
			ysta=yend;
			yend=t;
			t=xsta;
			xsta=xend;
			xend=t;
		}   
		for(x=xsta;x<=xend;x++)  /*��x��Ϊ��׼*/ 
		{
			y =(uint32_t)(x-xsta)*(yend-ysta)/(xend-xsta)+ysta;
			LCD_DrawPoint(x,y,POINT_COLOR); 
		}
	} 
} 


/*********************************************************************************
** ������:              void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend)
** ����:                ��ָ�����򻭾���
** �������:            xsta X��ʼ����
**										  ysta Y��ʼ����
**										  xend X��ʼ����
**										  yend Y�յ�����		
** �������:            ��
** ����ֵ:              ��
*********************************************************************************/
void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t POINT_COLOR)
{
	LCD_DrawLine(xsta,ysta,xend,ysta,POINT_COLOR);
	LCD_DrawLine(xsta,ysta,xsta,yend,POINT_COLOR);
	LCD_DrawLine(xsta,yend,xend,yend,POINT_COLOR);
	LCD_DrawLine(xend,ysta,xend,yend,POINT_COLOR);
} 
/*********************************************************************************************************
** ������:              void Draw_Circle(uint16_t x0,uint16_t y0,uint16_t r)
** ����:                ��Բ
** �������:            x0,y0,Բ������
**										  r:�뾶	
** �������:            ��
** ����ֵ:              ��
*********************************************************************************************************/ 
void Draw_Circle(uint8_t x0,uint16_t y0,uint8_t r,uint16_t Color)
{

	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b,Color);             //5
 		LCD_DrawPoint(x0+b,y0-a,Color);             //0           
		LCD_DrawPoint(x0+b,y0+a,Color);             //4               
		LCD_DrawPoint(x0+a,y0+b,Color);             //6 
		LCD_DrawPoint(x0-a,y0+b,Color);             //1       
 		LCD_DrawPoint(x0-b,y0+a,Color);             
		LCD_DrawPoint(x0-a,y0-b,Color);             //2             
  		LCD_DrawPoint(x0-b,y0-a,Color);             //7     	         
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}

} 
