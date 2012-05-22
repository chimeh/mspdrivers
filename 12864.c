/*****************************************
*使用到的IO口宏定义，修改IO口只需修改此处*
*****************************************/
#define PORT_DIR        P1DIR
#define PORT_OUT        P1OUT
#define PORT_IN         P1IN

#define CS      (1<<4)  //RS    //Px0
#define SID     (1<<5)  //RW    //px1
#define SCLK    (1<<6)  //E     //Px2

#include <intrinsics.h>
#define CPU_F 4000000 //CPU频率
#define delay_us(count) __delay_cycles(0.25*count * CPU_F/1000000)
#define delay_ms(count) __delay_cycles(0.25*count * CPU_F/1000)
#define delay_s(count)  __delay_cycles(0.25*count * CPU_F)
#include "12864.h"
#include <msp430g2231.h>

/**********初始化使用到的IO口*******************/
void IO_Init(void)   //初始化端口
{
 PORT_DIR|=CS;
 PORT_DIR|=SID;//设置为输出
 PORT_DIR|=SCLK;
}

/************************************
*函数功能：串行发送一个字节的数据
*参数：要发送的数据
*************************************/
void SendByte(uchar Byte)
{
        uchar i;
        for(i=0;i<8;i++)
        {
                PORT_OUT&=~SCLK;                //SCLK=0
                if(Byte & 0x80)
                        PORT_OUT|=SID;  //SID=1;
                else
                        PORT_OUT&=~SID; //SID=0;
                PORT_OUT|=SCLK;         //SCLK=1;
                Byte=Byte<<1;
        }
}

/***********************************
*函数功能：串行接收一个字节数据
*返回值：  接收到的数据
***********************************/
uchar ReceiveByte(void)
{
        uchar i,tmp1=0,tmp2=0;
        PORT_DIR&=~SID; //设置为输入
        for(i=0;i<8;i++)
        {
                PORT_OUT&=~SCLK;
                PORT_OUT|=SCLK;
                if(PORT_IN & SID)
                tmp1|=(0x80>>i);  //data=1;
        }
        for(i=0;i<8;i++)
        {       tmp2=tmp2<<1;
                PORT_OUT&=~SCLK;
                PORT_OUT|=SCLK;
                if(PORT_IN & SID)
                tmp2|=(0x80>>i);  //data=1;

        }
        PORT_DIR|=SID;  //恢复设置为输出
        return  (tmp1&0xf0)+(tmp2&0x0f);
}

/******************************************
*函数功能：判读LCD是否处于忙状态
*******************************************/
void CheckBusy(void)
{       PORT_OUT|=CS;
        do SendByte(0xfc);            //11111 10 0      RW=1,RS=0读忙标志
        while(0x80&ReceiveByte());    //BF(.7)=1 Busy
        PORT_OUT&=~CS;
}


/******************************************
*函数功能：发送控制命令
*参数：       发送的命令
*******************************************/
void WriteCOM (int COM)
{
        CheckBusy();
        PORT_OUT|=CS;                   //CS=1
        SendByte(0xf8);                 //11111 00 0    RW=0,RS=0写指令
        SendByte(0xf0 & COM);           //发送高四位
        SendByte(0xf0 & COM<<4);        //发送低四位
        PORT_OUT&=~CS;                  //CS=0
}

/********************************************
*函数功能： 发送显示数据
*参数：        要显示的数据
*********************************************/
void WriteData(int DATA)
{
        CheckBusy();
        PORT_OUT|=CS;                   //CS=1
        SendByte(0xfa);                 //11111 01 0    RW=0,RS=1写数据
        SendByte(0xf0 & DATA);          //发送高四位
        SendByte(0xf0 & DATA<<4);       //发送第四位
        PORT_OUT&=~CS;                  //CS=1

}
void WriteLine(int ADDR,int Data)//ADDR是DDRAM中的寄存器。 Data存放的是半角的字符，也就是ASCII码。
{
        uchar count=0;
        WriteCOM(ADDR);
        for(count=0;count<16;count++)
        {
                WriteData(Data);
        }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//      下面是供用户调用的函数。
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
/******************************************
*函数功能:  LCD初始化
*******************************************/
void LCD_Init(void)
{
        IO_Init();                              //初始化IO
        delay_ms(40);
        WriteCOM(0x30);
        delay_us(100);
        WriteCOM(0x30);
        delay_us(37);
        WriteCOM(0x0c);
        delay_us(100);
        WriteCOM(0x01);
        delay_ms(10);
        WriteCOM(0x06);
}
/******************************************
*函数功能:  清整屏幕
*******************************************/
void LCD_Clear(void)
{
        WriteCOM(0x01);
        WriteCOM(0x34);
        WriteCOM(0x30);
}
/******************************************
*函数功能:  设置显示位置，X 表示行，Y-表示列
*******************************************/
void LCD_SetTextPos(uint X,uint Y)
{
        uchar Addr;
        switch (X)
        {
                case 0:
                        Addr = 0x80+Y;
                        break;
                case 1:
                        Addr = 0x90+Y;
                        break;
                case 2:
                        Addr = 0x88+Y;
                        break;
                case 3:
                        Addr = 0x98+Y;
                        break;
        }
        WriteCOM(Addr);
}
//===========================================================
//函数功能:         显示一个半角字符。
//参数：           显示一个字符的ASCII码值
//返回值：          无
//===========================================================
#define LCD_PutChar(CharCode)   WriteData(CharCode)
//======================================================
// 函数名称：        LCD701_Print
// 格式：          void LCD701_Print(unsigned char *String)
// 实现功能：        显示一个字符串（可以由半角和全角字符混合组成）
// 参数：          String - 字符串的首地址
// 返回值：         无
//======================================================
void LCD_Print(const uchar * String)
{
        uchar Text;
        /*
         这里有一个常见的错误  while (Text = (*String ++) != '\0')
        */
        while( (Text = *String ++) != '\0')
          WriteData(Text);
}



void LCD_DisNum(uint NumToBeDis)
{
        uchar IntToDis[6];
        IntToDis[0] = NumToBeDis / 10000 + '0';
        IntToDis[1] = NumToBeDis % 10000 / 1000 + '0';
        IntToDis[2] = NumToBeDis % 1000 /100 + '0';
        IntToDis[3] = NumToBeDis % 100/10 + '0';
        IntToDis[4] = NumToBeDis % 10 + '0';
        IntToDis[5] = '\0';
        LCD_Print(IntToDis);
}
