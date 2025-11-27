/**
 * SPI.c
 *
 *  Created on: May 2, 2023
 *      Author: benes
 * Defines OLED panel API
 */

#include <apis/SPI_API.h>
#include <ic_drivers/fonts.c>
#include <ic_drivers/SSD1309.h>
#include "pin_mux.h"
#include "fsl_gpio.h"

unsigned char buffer[buffer_size];

void RES_Clr(void)
{
	GPIO_PortClear(GPIO, SPI0_RST_PORT,
                   1U << SPI0_RST_PIN);
}

void RES_Set(void)
{

	GPIO_PortSet(GPIO, SPI0_RST_PORT, \
                   1U << SPI0_RST_PIN);
}

void DC_OLED_Write(uint8_t data)
{
	GPIO_PinWrite(GPIO, SPI0_DC_PORT, SPI0_DC_PIN, data);
}

void CyDelay(uint32_t t)
{
	uint32_t i;
	for(i=0; i<1000*t; i++)
	{
	__asm volatile ("nop");
	}
}

void swap(signed int *a, signed int *b)
{
    signed int temp = 0x0000;

    temp = *b;
    *b = *a;
    *a = temp;
}


void OLED_init()
{
    //setup_GPIOs();
    //SPIM_OLED_Start();
    
    OLED_reset_sequence();
    
    OLED_write((Set_Display_ON_or_OFF_CMD + Display_OFF) , CMD);;
    
    OLED_write(Set_Display_Clock_CMD, CMD);
    OLED_write(0x80, CMD);
    
    OLED_write(Set_Multiplex_Ratio_CMD, CMD);
    OLED_write(0x3F, CMD);
    
    OLED_write(Set_Display_Offset_CMD, CMD);
    OLED_write(0x00, CMD);
    
    OLED_write((Set_Display_Start_Line_CMD | 0x00), CMD);
    
    OLED_write(Set_Charge_Pump_CMD, CMD);
    OLED_write((Set_Higher_Column_Start_Address_CMD | Enable_Charge_Pump), CMD);

    OLED_write(Set_Memory_Addressing_Mode_CMD, CMD);
    OLED_write(Page_Addressing_Mode, CMD);
    
    OLED_write((Set_Segment_Remap_CMD | Column_Address_0_Mapped_to_SEG127), CMD);
    
    OLED_write((Set_COM_Output_Scan_Direction_CMD | Scan_from_COM63_to_0), CMD);
    
    OLED_write(Set_Common_HW_Config_CMD, CMD);
    OLED_write(0x12, CMD);

    OLED_write(Set_Contrast_Control_CMD, CMD);
    OLED_write(0xCF, CMD);
    
    OLED_write(Set_Pre_charge_Period_CMD, CMD);
    OLED_write(0xF1, CMD);
    
    OLED_write(Set_VCOMH_Level_CMD, CMD);
    OLED_write(0x40, CMD);

    OLED_write((Set_Entire_Display_ON_CMD | Normal_Display), CMD);

    OLED_write((Set_Normal_or_Inverse_Display_CMD | Non_Inverted_Display), CMD);
    
    OLED_write((Set_Display_ON_or_OFF_CMD + Display_ON) , CMD);
    
    OLED_gotoxy(0, 0);
    
    OLED_clear_buffer();
    OLED_clear_screen();
}


void OLED_reset_sequence()
{
    //delay_ms(40);
    CyDelay(40);
    //RST = LOW
    //RST_OLED_Write(0);
    RES_Clr();
    //delay_ms(40);
    CyDelay(40);
    //RST = HIGH;
    //RST_OLED_Write(1);
    RES_Set();
}


void OLED_write(unsigned char value, unsigned char type)
{
    //DC = (type & 0x01);
    DC_OLED_Write(type & 0x01);
    //CS = LOW;
    //CS_OLED_Write(0);
    //SPI1_Write(value);
    /*
    SPIM_OLED_WriteTxData(value);
    do{
    }while(!(SPIM_OLED_ReadTxStatus()&(SPIM_OLED_STS_SPI_DONE)));
    */
    SPI_MasterStartTransfer(SPI0, value);
    //CS = HIGH;
    //CS_OLED_Write(1);
}  


void OLED_gotoxy(unsigned char x_pos, unsigned char y_pos)
{                                    
    OLED_write((Set_Page_Start_Address_CMD + y_pos), CMD);
    OLED_write(((x_pos & 0x0F) | Set_Lower_Column_Start_Address_CMD), CMD);
    OLED_write((((x_pos & 0xF0) >> 0x04) | Set_Higher_Column_Start_Address_CMD), CMD);
}


void OLED_fill(unsigned char bmp_data) 
{                                                     
    unsigned char x_pos = 0x00;
    unsigned char page = 0x00;

    for(page = y_min; page < y_max; page++)
    {
        OLED_write((Set_Page_Start_Address_CMD + page), CMD);
        OLED_write(Set_Lower_Column_Start_Address_CMD, CMD);
        OLED_write(Set_Higher_Column_Start_Address_CMD, CMD);

        for(x_pos = x_min; x_pos < x_max; x_pos++)
        {
            OLED_write(bmp_data, DAT);
        }
    }
} 


void OLED_clear_screen()
{ 
    OLED_fill(0x00);
}


void OLED_clear_buffer()
{
     unsigned int s = 0x00;

     for(s = 0; s < buffer_size; s++)
     {
          buffer[s] = 0x00;
     }
}


void OLED_cursor(unsigned char x_pos, unsigned char y_pos)
{         
    unsigned char i = 0x00;

    if(y_pos != 0x00)
    {
        if(x_pos == 1)
        {
            OLED_gotoxy(0x00, (y_pos + 0x02));
        }
        else
        {
            OLED_gotoxy((0x50 + ((x_pos - 0x02) * 0x06)), (y_pos + 0x02));
        }

        for(i = 0; i < 6; i++)
        {
            OLED_write(0xFF, DAT);
        }
    }
}


void OLED_print_Image(const unsigned char *bmp, unsigned char pixel)
{
    unsigned char x_pos = 0x00;
    unsigned char page = 0x00;

    if(pixel != OFF)
    {
        pixel = 0xFF;
    }
    else
    {
        pixel = 0x00;
    }

    for(page = 0; page < y_max; page++)
    {
         OLED_gotoxy(x_min, page);
         for(x_pos = x_min; x_pos < x_max; x_pos++)
         {
            OLED_write((*bmp++ ^ pixel), DAT);
         }
     }
}


void OLED_draw_bitmap(unsigned char xb, unsigned char yb, unsigned char xe, unsigned char ye, unsigned char *bmp_img)
{
    unsigned int s = 0x0000;
    unsigned char x_pos = 0x00;
    unsigned char y_pos = 0x00;

    for(y_pos = yb; y_pos <= ye; y_pos++)
    {
        OLED_gotoxy(xb, y_pos);
        for(x_pos = xb; x_pos < xe; x_pos++)
        {
            OLED_write(bmp_img[s], DAT);
            s++;
        }
    }
}
       
                    
void OLED_print_char(unsigned char x_pos, unsigned char y_pos, unsigned char ch)
{
    unsigned char chr = 0x00;
    unsigned char s = 0x00;

    chr = (ch - 32);

    if(x_pos > (x_max - 6))
    {
        x_pos = 0;
        y_pos++;
    }
    
    OLED_gotoxy(x_pos, y_pos);

    for(s = 0x00; s < 0x06; s++)
    {
        OLED_write(font_regular[chr][s], DAT);
    }
}
  

//void OLED_print_string(unsigned char x_pos, unsigned char y_pos, unsigned char *ch)
void OLED_print_string(unsigned char x_pos, unsigned char y_pos, char *ch)
{
    unsigned char chr = 0x00;
    unsigned char i = 0x00;
    unsigned char j = 0x00;

    while(ch[j] != '\0')
    {
        chr = (ch[j] - 32); //ASCII - space = font[chr]

        if(x_pos > (x_max - 0x06))
        {
            x_pos = 0x00;
            y_pos++;
        }
        OLED_gotoxy(x_pos, y_pos);

        for(i = 0x00; i < 0x06; i++)
        {
            OLED_write(font_regular[chr][i], DAT);
        }

        j++;
        x_pos += 6;
     }
}    
                                             
                      
void OLED_print_chr(unsigned char x_pos, unsigned char y_pos, signed int value)
{                                             
    unsigned char ch = 0x00;

    if(value < 0x00)
    {
        OLED_print_char(x_pos, y_pos, '-');
        value = -value;
    }
    else
    {
        OLED_print_char(x_pos, y_pos,' ');
    }

     if((value > 99) && (value <= 999))
     {
         ch = (value / 100);
         OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));
         ch = ((value % 100) / 10);
         OLED_print_char((x_pos + 12), y_pos , (0x30 + ch));
         ch = (value % 10);
         OLED_print_char((x_pos + 18), y_pos , (0x30 + ch));
     }
     else if((value > 9) && (value <= 99))
     {
         ch = ((value % 100) / 10);
         OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));
         ch = (value % 10);
         OLED_print_char((x_pos + 12), y_pos , (0x30 + ch));
         OLED_print_char((x_pos + 18), y_pos , 0x20);
     }
     else if((value >= 0) && (value <= 9))
     {
         ch = (value % 10);
         OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));
         OLED_print_char((x_pos + 12), y_pos , 0x20);
         OLED_print_char((x_pos + 18), y_pos , 0x20);
     }
}


void OLED_print_int(unsigned char x_pos, unsigned char y_pos, signed long value)
{ 
    unsigned char ch = 0x00;

    if(value < 0)
    {
        OLED_print_char(x_pos, y_pos, '-');
        value = -value;
    }
    else
    {
        OLED_print_char(x_pos, y_pos,' ');
    }

    if(value > 9999)
    {
        ch = (value / 10000);
        OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));

        ch = ((value % 10000)/ 1000);
        OLED_print_char((x_pos + 12), y_pos , (0x30 + ch));

        ch = ((value % 1000) / 100);
        OLED_print_char((x_pos + 18), y_pos , (0x30 + ch));

        ch = ((value % 100) / 10);
        OLED_print_char((x_pos + 24), y_pos , (0x30 + ch));

        ch = (value % 10);
        OLED_print_char((x_pos + 30), y_pos , (0x30 + ch));
    }

    else if((value > 999) && (value <= 9999))
    {
        ch = ((value % 10000)/ 1000);
        OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));

        ch = ((value % 1000) / 100);
        OLED_print_char((x_pos + 12), y_pos , (0x30 + ch));

        ch = ((value % 100) / 10);
        OLED_print_char((x_pos + 18), y_pos , (0x30 + ch));

        ch = (value % 10);
        OLED_print_char((x_pos + 24), y_pos , (0x30 + ch));
        OLED_print_char((x_pos + 30), y_pos , 0x20);
    }
    else if((value > 99) && (value <= 999))
    {
        ch = ((value % 1000) / 100);
        OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));

        ch = ((value % 100) / 10);
        OLED_print_char((x_pos + 12), y_pos , (0x30 + ch));

        ch = (value % 10);
        OLED_print_char((x_pos + 18), y_pos , (0x30 + ch));
        OLED_print_char((x_pos + 24), y_pos , 0x20);
        OLED_print_char((x_pos + 30), y_pos , 0x20);
    }
    else if((value > 9) && (value <= 99))
    {
        ch = ((value % 100) / 10);
        OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));

        ch = (value % 10);
        OLED_print_char((x_pos + 12), y_pos , (0x30 + ch));
        
        OLED_print_char((x_pos + 18), y_pos , 0x20);
        OLED_print_char((x_pos + 24), y_pos , 0x20);
        OLED_print_char((x_pos + 30), y_pos , 0x20);
    }
    else
    {
        ch = (value % 10);
        OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));
        OLED_print_char((x_pos + 12), y_pos , 0x20);
        OLED_print_char((x_pos + 18), y_pos , 0x20);
        OLED_print_char((x_pos + 24), y_pos , 0x20);
        OLED_print_char((x_pos + 30), y_pos , 0x20);
    }
}                                                      


void OLED_print_decimal(unsigned char x_pos, unsigned char y_pos, unsigned int value, unsigned char points)
{
    unsigned char ch = 0x00;
    unsigned char tmp = 0x00;

    OLED_print_char(x_pos, y_pos, '.');
    ch = (value / 1000);
   if (points == 1) { //rounding
	   tmp = (value / 100)-ch*10;
	   if (tmp >= 5) {
		   ch++;
	   }
   }

    OLED_print_char((x_pos + 6), y_pos , (0x30 + ch));

    if(points > 1)
    {
        ch = ((value % 1000) / 100);
        OLED_print_char((x_pos + 12), y_pos , (0x30 + ch));


        if(points > 2)
        {
            ch = ((value % 100) / 10);
            OLED_print_char((x_pos + 18), y_pos , (0x30 + ch));

            if(points > 3)
            {
                ch = (value % 10);
                OLED_print_char((x_pos + 24), y_pos , (0x30 + ch));
            }
        }
    }
}
 
                             
void OLED_print_float(unsigned char x_pos, unsigned char y_pos, float value, unsigned char points) 
{
    signed long tmp = 0x00;

    tmp = value;
    OLED_print_int(x_pos, y_pos, tmp);
    tmp = ((value - tmp) * 10000);
    
    if(tmp < 0)
    {
       tmp = -tmp;
    }

    if((value >= 10000) && (value < 100000))
    {
        OLED_print_decimal((x_pos + 36), y_pos, tmp, points);
    }
    else if((value >= 1000) && (value < 10000))
    {
        OLED_print_decimal((x_pos + 30), y_pos, tmp, points);
    }
    else if((value >= 100) && (value < 1000))
    {
        OLED_print_decimal((x_pos + 24), y_pos, tmp, points);
    }
    else if((value >= 10) && (value < 100))
    {
        OLED_print_decimal((x_pos + 18), y_pos, tmp, points);
    }
    else if(value < 10)
    {
        OLED_print_decimal((x_pos + 12), y_pos, tmp, points);
        if((value) < 0)
        {
            OLED_print_char(x_pos, y_pos, '-');
        }
        else
        {
            OLED_print_char(x_pos, y_pos, ' ');
        }
    }
}


void Draw_Pixel(unsigned char x_pos, unsigned char y_pos, unsigned char colour)
{
    unsigned char value = 0x00;
    unsigned char page = 0x00;
    unsigned char bit_pos = 0x00;
    
    page = (y_pos / y_max);
    bit_pos = (y_pos - (page * y_max));
    value = buffer[((page * x_max) + x_pos)];
    
    if((colour & 0x01) != 0)
    {
        value |= (1 << bit_pos);
    }
    else
    {
        value &= (~(1 << bit_pos));
    }
    
    buffer[((page * x_max) + x_pos)] = value;
    OLED_gotoxy(x_pos, page);
    OLED_write(value, DAT);
}


void Draw_Line(signed int x1, signed int y1, signed int x2, signed int y2, unsigned char colour)
{
    signed int dx = 0x0000;
    signed int dy = 0x0000;
    signed int stepx = 0x0000;
    signed int stepy = 0x0000;
    signed int fraction = 0x0000;

    dy = (y2 - y1);
    dx = (x2 - x1);

    if (dy < 0)
    {
        dy = -dy;
        stepy = -1;
    }
    else
    {
        stepy = 1;
    }

    if (dx < 0)
    {
        dx = -dx;
        stepx = -1;
    }
    else
    {
        stepx = 1;
    }

    dx <<= 1;
    dy <<= 1;

    Draw_Pixel(x1, y1, colour);

    if(dx > dy)
    {
        fraction = (dy - (dx >> 1));
        while (x1 != x2)
        {
            if(fraction >= 0)
            {
                y1 += stepy;
                fraction -= dx;
            }
            
            x1 += stepx;
            fraction += dy;

            Draw_Pixel(x1, y1, colour);
        }
    }
    else
    {
        fraction = (dx - (dy >> 1));
        while (y1 != y2)
        {
            if (fraction >= 0)
            {
                x1 += stepx;
                fraction -= dy;
            }
            
            y1 += stepy;
            fraction += dx;
            
            Draw_Pixel(x1, y1, colour);
        }
    }
}


void Draw_V_Line(signed int x1, signed int y1, signed int y2, unsigned colour)
{
    //signed int pos = 0;
    //signed int temp = 0;

    if(y1 > y2)
    {
       swap(&y1, &y2);
    }

    while(y2 > (y1 - 1))
    {
        Draw_Pixel(x1, y2, colour);
        y2--;
    }
}


void Draw_H_Line(signed int x1, signed int x2, signed int y1, unsigned colour)
{
    //signed int pos = 0;
    //signed int temp = 0;

    if(x1 > x2)
    {
       swap(&x1, &x2);
    }

    while(x2 > (x1 - 1))
    {
        Draw_Pixel(x2, y1, colour);
        x2--;
    }
}


void Draw_Triangle(signed int x1, signed int y1, signed int x2, signed int y2, signed int x3, signed int y3, unsigned char fill, unsigned int colour)
{
    signed int a = 0;
    signed int b = 0;
    signed int sa = 0;
    signed int sb = 0;
    signed int yp = 0;
    signed int last = 0;
    signed int dx12 = 0;
    signed int dx23 = 0;
    signed int dx13 = 0;
    signed int dy12 = 0;
    signed int dy23 = 0;
    signed int dy13 = 0;

    switch(fill)
    {
        case YES:
        {
            if(y1 > y2)
            {
                swap(&y1, &y2);
                swap(&x1, &x2);
            }
            if(y2 > y3)
            {
                swap(&y3, &y2);
                swap(&x3, &x2);
            }
            if(y1 > y2)
            {
                swap(&y1, &y2);
                swap(&x1, &x2);
            }

            if(y1 == y3)
            {
                a = b = x1;

                if(x2 < a)
                {
                    a = x2;
                }
                else if(x2 > b)
                {
                    b = x2;
                }
                if(x2 < a)
                {
                    a = x3;
                }
                else if(x3 > b)
                {
                    b = x3;
                }

                Draw_H_Line(a, (a + (b - (a + 1))), y1, colour);
                return;
            }

            dx12 = (x2 - x1);
            dy12 = (y2 - y1);
            dx13 = (x3 - x1);
            dy13 = (y3 - y1);
            dx23 = (x3 - x2);
            dy23 = (y3 - y2);
            sa = 0,
            sb = 0;

            if(y2 == y3)
            {
                last = y2;
            }
            else
            {
                last = (y2 - 1);
            }

            for(yp = y1; yp <= last; yp++)
            {
                a = (x1 + (sa / dy12));
                b = (x1 + (sb / dy13));
                sa += dx12;
                sb += dx13;
                if(a > b)
                {
                    swap(&a, &b);
                }
                Draw_H_Line(a, (a + (b - (a + 1))), yp, colour);
            }

            sa = (dx23 * (yp - y2));
            sb = (dx13 * (yp - y1));
            for(; yp <= y3; yp++)
            {
                a = (x2 + (sa / dy23));
                b = (x1 + (sb / dy13));
                sa += dx23;
                sb += dx13;

                if(a > b)
                {
                    swap(&a, &b);
                }
                Draw_H_Line(a, (a + (b - (a + 1))), yp, colour);
            }


            break;
        }
        default:
        {
            Draw_Line(x1, y1, x2, y2, colour);
            Draw_Line(x2, y2, x3, y3, colour);
            Draw_Line(x3, y3, x1, y1, colour);
            break;
        }
    }
}


void Draw_Rectangle(signed int x1, signed int y1, signed int x2, signed int y2, unsigned char fill, unsigned char colour, unsigned char type)
{
     unsigned short i = 0x00;
     unsigned short xmin = 0x00;
     unsigned short xmax = 0x00;
     unsigned short ymin = 0x00;
     unsigned short ymax = 0x00;

     if(fill != 0)
     {
        if(x1 < x2)
        {
           xmin = x1;
           xmax = x2;
        }
        else
        {
           xmin = x2;
           xmax = x1;
        }

        if(y1 < y2)
        {
           ymin = y1;
           ymax = y2;
        }
        else
        {
           ymin = y2;
           ymax = y1;
        }

        for(; xmin <= xmax; ++xmin)
        {
           for(i = ymin; i <= ymax; ++i)
           {
               Draw_Pixel(xmin, i, colour);
           }
         }
     }

     else
     {
        Draw_Line(x1, y1, x2, y1, colour);
        Draw_Line(x1, y2, x2, y2, colour);
        Draw_Line(x1, y1, x1, y2, colour);
        Draw_Line(x2, y1, x2, y2, colour);
     }
     
     if(type != SQUARE)
     {
         Draw_Pixel(x1, y1, ~colour);
         Draw_Pixel(x1, y2, ~colour);
         Draw_Pixel(x2, y1, ~colour);
         Draw_Pixel(x2, y2, ~colour);
     }
}


void Draw_Circle(signed int xc, signed int yc, signed int radius, unsigned char fill, unsigned char colour)
{
   signed int a = 0x0000;
   signed int b = 0x0000;
   signed int P = 0x0000;

   b = radius;
   P = (1 - b);

   do
   {
        if(fill != 0)
        {
           Draw_Line((xc - a), (yc + b), (xc + a), (yc + b), colour);
           Draw_Line((xc - a), (yc - b), (xc + a), (yc - b), colour);
           Draw_Line((xc - b), (yc + a), (xc + b), (yc + a), colour);
           Draw_Line((xc - b), (yc - a), (xc + b), (yc - a), colour);
        }
        else
        {
           Draw_Pixel((xc + a), (yc + b), colour);
           Draw_Pixel((xc + b), (yc + a), colour);
           Draw_Pixel((xc - a), (yc + b), colour);
           Draw_Pixel((xc - b), (yc + a), colour);
           Draw_Pixel((xc + b), (yc - a), colour);
           Draw_Pixel((xc + a), (yc - b), colour);
           Draw_Pixel((xc - a), (yc - b), colour);
           Draw_Pixel((xc - b), (yc - a), colour);
        }

        if(P < 0)
        {
           P += (3 + (2 * a++));
        }
        else
        {
           P += (5 + (2 * ((a++) - (b--))));
        }
    }while(a <= b);
}



// ******************* SCOPE *********************
#define Ym 47
#define Ypix 63
#define Xpix 128
volatile double val_buf[Xpix];

typedef struct coor_Y
{
    int value;
    unsigned char valid;
}coor_Y;
volatile coor_Y buf[Xpix];
//volatile int buf[Xpix];

volatile double MaxY;
volatile double MinY;

typedef struct DP_scale
{
    double Amp;
    double k;
    double max;
    double min;
}DP_scale;


void ReScale(volatile DP_scale *ScopeScale)
{
    int i;
    MaxY = -10e10;
    MinY = 10e10;
    for(i = 0; i < Xpix; i++)
    {
        if(val_buf[i] > MaxY)
        {
            MaxY = val_buf[i];
        }
        if(val_buf[i] < MinY)
        {
            MinY = val_buf[i];
        }                  
    }
    ScopeScale->Amp = MaxY-MinY;
    ScopeScale->k = Ym/(ScopeScale->Amp);
    ScopeScale->max = MaxY;
    ScopeScale->min = MinY;
}

//(C) Zoltán Benesóczky 2019
//periodikusan meg kell hívni az időfüggvény Y(t) aktuálisan megjelenítendő Yti értékével
//megjeleníti a függvényt pontokkal vagy vonallal összekötött pontokkal
//a megjelenítendő idő léptéke módosítható
//megjelenítés közben időnként újraskáláz, hogy a megjeleníthető területet teljesen kihasználja
//paraméterek:
//value: aktuális érték (Yti),
//dsamp: minden dsamp-adik mintát jelenít csak meg (a displayen megjeleníthető idő növelése)
//tcl_num: újra skálázás minden tcl_num számú megjelenített teljes képernyő után
//pl: pl=1 pont rajzolás pl=0 vonal rajzolás (előző és aktuális pont összekötése) 
//inv: ivertált jel rajzolása
//reset: reset = 1 alapállapotba állítás. Egyszer mindenképpen meg kell hívni az elején így!
//Reset esetén csak a dsampi paraméter érdekes.
void Scope(double value, int dsamp, int tcl_num, uint8_t pl, uint8_t inv, uint8_t res)
{
    //ScopeState:
    #define MaxMin 0
    #define Norm 1
    
    //#define tcl0 4
    /*
    #define Ym 47
    #define Ypix 63
    #define Xpix 128
    */    
    static volatile uint8_t ScopeState;
    static volatile uint8_t t, tprev, tcl;
    //static volatile int buf[Xpix];
    /*
    static double val_buf[Xpix];
    static double MaxY;
    static double MinY;
    volatile static double Amp;
    volatile static double k;
    */
    static volatile DP_scale scale;
    static  volatile int dsampi;
    static volatile coor_Y y, y_last;
    static volatile double yf;
    static volatile double val;
    static volatile unsigned char first_draw;
    val = value;
    if(dsamp <= 0)
    {
        dsamp = 1;
    }
    
    if(res)
    {
        OLED_clear_buffer();
        OLED_fill(0x00);
        ScopeState = MaxMin;
        first_draw = 1;
        tcl = 0;
        //MaxY = -10e10;
        scale.max = -10e10;
        MinY = 10e10;
        scale.min = 10e10;
        scale.Amp = 0;
        scale.k = 0;
        /*
        if(dsamp == 0)
        {
            dsamp = 1;
        } 
        */
        dsampi = 0;
        for(t = 0; t < Xpix; t++)
        {
            buf[t].valid = 0;
        }
        t = 0;
    }
    else
    {
        if(dsampi >= (dsamp - 1))
        {
            dsampi = 0;
            
            switch (ScopeState)
            {
            case MaxMin:    //first initialise scale values
                val_buf[t] = val;
                t++;
                if(t < Xpix)
                {            
                    
                    //if(val > MaxY)
                    if(val > scale.max)
                    {
                        //MaxY = val;
                        scale.max = val;
                    }
                    //if(val < MinY)
                    if(val < scale.min)
                    {
                        //MinY = val;
                        scale.min = val;
                    }                      
                }
                if(t == Xpix)   //tele a képernyő puffer
                {
                    //scale.Amp = MaxY-MinY;
                    scale.Amp = scale.max-scale.min;
                    scale.k = Ym/scale.Amp;
                    t = 0;
                    tprev = -1;
                    ScopeState = Norm;
                }        
                break;
            case Norm:
                val_buf[t] = val; //save original double value
                //yf = scale.k*(val - MinY); //scale to display,
                yf = scale.k*(val-scale.min);
                /*
                if(t > 0)
                {
                    y_last = y;
                }
                */
                if(first_draw & (t > 0))
                {
                    y_last = y;                
                }
                else
                {
                    if(!first_draw)
                    {
                        y_last = y;
                    }
                }
                
                y.value = (int)(yf +0.5);  //and convert to integer
                if(inv)
                {
                    y.value = Ym-y.value;
                }
                //Draw_Pixel(unsigned char x_pos, unsigned char y_pos, unsigned char colour);
                //First clear the old y in x_pos
                if(!first_draw)
                {
                    if(pl)
                    {
                        Draw_Pixel(t,Ypix-buf[t].value,NO);
                    }
                    else
                    {
                        //Draw_Line(signed int x1, signed int y1, signed int x2, signed int y2, unsigned char colour);
                        //if((t > 0)&(tprev >=0))
                        if((t > 0)&&(t > tprev))
                        {
                            //Draw_Line(t-1, Ypix-buf[t-1],t, Ypix-buf[t],NO);
                            if((buf[tprev].valid)&&(buf[t].valid))
                            {
                                Draw_Line(tprev, Ypix-buf[tprev].value,t, Ypix-buf[t].value,NO);
                            }
                        }
                    }
                }
                //Then draw the new y in x_pos
                if((y.value >=0)&&(y.value <=Ym))  //if y is within the display area
                {   
                    y.valid = 1;
                    if(pl)
                    {
                        Draw_Pixel(t,Ypix-y.value,YES);                        
                        //buf[t].value = y;
                    }
                    else
                    {
                        if((t > 0)&&(t > tprev))
                        {
                            //Draw_Line(t-1, Ypix-buf[t-1],t, Ypix-y,YES);
                            //if(buf[tprev].valid)
                            if(y_last.valid)
                            {
                                //Draw_Line(tprev, Ypix-buf[tprev].value,t, Ypix-y,YES);
                                Draw_Line(tprev, Ypix-y_last.value,t, Ypix-y.value,YES);
                            }
                        }                                                                             
                    }                    
                    //buf[t] = y;           
                    if(!first_draw & (t == 0))  //Rescale every tcl_num screen
                    {
                        if(tcl < tcl_num-1) 
                        {
                            tcl++;
                        }
                        else
                        {
                         tcl = 0;
                         ReScale(&scale);   
                        }
                    } 
                    /*
                    tprev = t;                
                    t = (t+1)&(Xpix-1); //cyclic buffer
                    if(t == 0)
                    {
                        first_draw = 0;  //buff filled
                    } 
                    */
                }        
                else  //make new scale
                {
                    //ReScale(&scale);
                    y.valid = 0;                    
                }
                
                if(first_draw & (t > 0))
                {
                    buf[tprev] = y_last;
                }
                else
                {
                    if(!first_draw)
                    {
                      buf[tprev] = y_last;  
                    }
                }                
                
                tprev = t;                
                t = (t+1)&(Xpix-1); //cyclic buffer

                if(t == 0)
                {
                    first_draw = 0;  //buff filled
                }
               
                break;
            default:
                break;
            }
        }
        else
        {
            dsampi++;
        }       
    }
}
