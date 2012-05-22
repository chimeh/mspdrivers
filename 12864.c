/*****************************************
*ʹ�õ���IO�ں궨�壬�޸�IO��ֻ���޸Ĵ˴�*
*****************************************/
#define PORT_DIR        P1DIR
#define PORT_OUT        P1OUT
#define PORT_IN         P1IN

#define CS      (1<<4)  //RS    //Px0
#define SID     (1<<5)  //RW    //px1
#define SCLK    (1<<6)  //E     //Px2

#include <intrinsics.h>
#define CPU_F 4000000 //CPUƵ��
#define delay_us(count) __delay_cycles(0.25*count * CPU_F/1000000)
#define delay_ms(count) __delay_cycles(0.25*count * CPU_F/1000)
#define delay_s(count)  __delay_cycles(0.25*count * CPU_F)
#include "12864.h"
#include <msp430g2231.h>

/**********��ʼ��ʹ�õ���IO��*******************/
void IO_Init(void)   //��ʼ���˿�
{
 PORT_DIR|=CS;
 PORT_DIR|=SID;//����Ϊ���
 PORT_DIR|=SCLK;
}

/************************************
*�������ܣ����з���һ���ֽڵ�����
*������Ҫ���͵�����
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
*�������ܣ����н���һ���ֽ�����
*����ֵ��  ���յ�������
***********************************/
uchar ReceiveByte(void)
{
        uchar i,tmp1=0,tmp2=0;
        PORT_DIR&=~SID; //����Ϊ����
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
        PORT_DIR|=SID;  //�ָ�����Ϊ���
        return  (tmp1&0xf0)+(tmp2&0x0f);
}

/******************************************
*�������ܣ��ж�LCD�Ƿ���æ״̬
*******************************************/
void CheckBusy(void)
{       PORT_OUT|=CS;
        do SendByte(0xfc);            //11111 10 0      RW=1,RS=0��æ��־
        while(0x80&ReceiveByte());    //BF(.7)=1 Busy
        PORT_OUT&=~CS;
}


/******************************************
*�������ܣ����Ϳ�������
*������       ���͵�����
*******************************************/
void WriteCOM (int COM)
{
        CheckBusy();
        PORT_OUT|=CS;                   //CS=1
        SendByte(0xf8);                 //11111 00 0    RW=0,RS=0дָ��
        SendByte(0xf0 & COM);           //���͸���λ
        SendByte(0xf0 & COM<<4);        //���͵���λ
        PORT_OUT&=~CS;                  //CS=0
}

/********************************************
*�������ܣ� ������ʾ����
*������        Ҫ��ʾ������
*********************************************/
void WriteData(int DATA)
{
        CheckBusy();
        PORT_OUT|=CS;                   //CS=1
        SendByte(0xfa);                 //11111 01 0    RW=0,RS=1д����
        SendByte(0xf0 & DATA);          //���͸���λ
        SendByte(0xf0 & DATA<<4);       //���͵���λ
        PORT_OUT&=~CS;                  //CS=1

}
void WriteLine(int ADDR,int Data)//ADDR��DDRAM�еļĴ����� Data��ŵ��ǰ�ǵ��ַ���Ҳ����ASCII�롣
{
        uchar count=0;
        WriteCOM(ADDR);
        for(count=0;count<16;count++)
        {
                WriteData(Data);
        }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//      �����ǹ��û����õĺ�����
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
/******************************************
*��������:  LCD��ʼ��
*******************************************/
void LCD_Init(void)
{
        IO_Init();                              //��ʼ��IO
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
*��������:  ������Ļ
*******************************************/
void LCD_Clear(void)
{
        WriteCOM(0x01);
        WriteCOM(0x34);
        WriteCOM(0x30);
}
/******************************************
*��������:  ������ʾλ�ã�X ��ʾ�У�Y-��ʾ��
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
//��������:         ��ʾһ������ַ���
//������           ��ʾһ���ַ���ASCII��ֵ
//����ֵ��          ��
//===========================================================
#define LCD_PutChar(CharCode)   WriteData(CharCode)
//======================================================
// �������ƣ�        LCD701_Print
// ��ʽ��          void LCD701_Print(unsigned char *String)
// ʵ�ֹ��ܣ�        ��ʾһ���ַ����������ɰ�Ǻ�ȫ���ַ������ɣ�
// ������          String - �ַ������׵�ַ
// ����ֵ��         ��
//======================================================
void LCD_Print(const uchar * String)
{
        uchar Text;
        /*
         ������һ�������Ĵ���  while (Text = (*String ++) != '\0')
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
