#include <REGX52.H>
#include <intrins.h>

// 直接在这里定义引脚，不依赖 lcd.h 以防万一
#define LCD1602_DATAPINS P0
sbit LCD1602_E  = P2^7;
sbit LCD1602_RW = P2^5;
sbit LCD1602_RS = P2^6;

typedef unsigned char u8;
typedef unsigned int u16;

// --- 真正的延时函数（针对 11.0592MHz 或 12MHz） ---
void My_Delay1ms(u16 c) {
    u8 a, b;
    for (; c > 0; c--) {
        for (b = 199; b > 0; b--) {
            for (a = 1; a > 0; a--);
        }
    }
}

// --- 全局变量 ---
u8 IrValue[4];
u8 IrOK = 0;
char code hex_table[] = "0123456789ABCDEF";
u8 Disp[] = " Pechin Science ";
sbit IRIN = P3^2;

// --- LCD底层驱动 ---
void LcdWriteCom(u8 com) {
    LCD1602_E = 0;
    LCD1602_RS = 0; 
    LCD1602_RW = 0; 
    LCD1602_DATAPINS = com; 
    _nop_(); _nop_();
    LCD1602_E = 1; 
    My_Delay1ms(2); // 确保使能脉冲足够长
    LCD1602_E = 0;
}

void LcdWriteData(u8 dat) {
    LCD1602_E = 0;
    LCD1602_RS = 1; 
    LCD1602_RW = 0; 
    LCD1602_DATAPINS = dat; 
    _nop_(); _nop_();
    LCD1602_E = 1; 
    My_Delay1ms(2); 
    LCD1602_E = 0;
}

void My_LcdInit() {
    My_Delay1ms(20);    // 等待LCD彻底上电
    LcdWriteCom(0x38);  // 设置模式
    My_Delay1ms(5);
    LcdWriteCom(0x38); 
    LcdWriteCom(0x0C);  // 开显示
    LcdWriteCom(0x06); 
    LcdWriteCom(0x01);  // 清屏
    My_Delay1ms(5);
}

// --- 红外相关 ---
void delay_10us(u16 i) {
    while(i--);
}

void IrInit() {
    IT0 = 1; EX0 = 1; EA = 1; IRIN = 1;
}

void main() {
    u8 i;
    My_LcdInit(); // 使用我们自己写的初始化
    IrInit();     
    
    LcdWriteCom(0x80); 
    for(i=0; i<16; i++) {
        LcdWriteData(Disp[i]);
    }
    
    LcdWriteCom(0xC0); 
    LcdWriteData('W'); LcdWriteData('a'); LcdWriteData('i');
    LcdWriteData('t'); LcdWriteData('i'); LcdWriteData('n');
    LcdWriteData('g'); LcdWriteData('.'); LcdWriteData('.');

    while(1) {
        if(IrOK) {
            IrOK = 0;
            LcdWriteCom(0xC0); 
            LcdWriteData('A'); LcdWriteData(':');
            LcdWriteData(hex_table[IrValue[0] >> 4]);   
            LcdWriteData(hex_table[IrValue[0] & 0x0F]); 
            LcdWriteData(' '); 
            LcdWriteData('C'); LcdWriteData(':');
            LcdWriteData(hex_table[IrValue[2] >> 4]);   
            LcdWriteData(hex_table[IrValue[2] & 0x0F]); 
            LcdWriteData(' '); LcdWriteData('O'); LcdWriteData('K');
        }
    }
}

// --- 红外中断 ---
void ReadIr() interrupt 0 {
    u8 j, k, time;
    u16 err;
    delay_10us(700); 
    if(IRIN == 0) {
        err = 1000;
        while((IRIN == 0) && (err > 0)) { delay_10us(1); err--; }
        if(IRIN == 1) {
            err = 500;
            while((IRIN == 1) && (err > 0)) { delay_10us(1); err--; }
            for(k = 0; k < 4; k++) {
                for(j = 0; j < 8; j++) {
                    err = 60;
                    while((IRIN == 0) && (err > 0)) { delay_10us(1); err--; }
                    err = 500; time = 0;
                    while((IRIN == 1) && (err > 0)) {
                        delay_10us(10); time++; err--;
                        if(time > 30) return; 
                    }
                    IrValue[k] >>= 1;
                    if(time >= 8) IrValue[k] |= 0x80;
                }
            }
        }
        if(IrValue[2] != (u8)~IrValue[3]) return;
        IrOK = 1; 
    }
}