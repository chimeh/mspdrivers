#include <msp430g2231.h>
//#include "io430.h"

#ifndef         _S_LCD_DRV
#define         _S_LCD_DRV
typedef unsigned int uint;
typedef unsigned char uchar;
void Delayus(uint number);
void IO_Init(void);   //³õÊ¼»¯¶Ë¿Ú
void WriteCOM(int COM);
void WriteData(int DATA);
void WriteLine(int ADDR,int Data);

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetTextPos(uint Row,uint Col);

#define LCD_PutChar(CharCode)   WriteData(CharCode)

void LCD_Print(const uchar *String);
void LCD_DisNum(uint NumToBeDis);
#endif
