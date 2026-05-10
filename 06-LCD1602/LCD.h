#ifndef __LCD_H_
#define __LCD_H_

#include <REGX52.H>  // 只保留这一个头文件引用

//---重定义关键词---//
#ifndef uchar
#define uchar unsigned char
#endif
#ifndef uint
#define uint unsigned int
#endif

//---PIN 口定义 (根据A2原理图)---//
#define LCD1602_DATAPINS P0
sbit LCD1602_E  = P2^7;
sbit LCD1602_RW = P2^5;
sbit LCD1602_RS = P2^6;

//---函数声明---//
void LcdInit();
void LcdWriteCom(uchar com);
void LcdWriteData(uchar dat);
void Lcd1602_Delay1ms(uint c);

#endif
