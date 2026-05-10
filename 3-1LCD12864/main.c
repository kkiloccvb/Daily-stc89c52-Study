#include <REGX52.H>
#include <intrins.h>

typedef unsigned char u8;
typedef unsigned int u16;

// --- 引脚定义 ---
sbit LCD_CS   = P1^0;
sbit LCD_RST  = P1^1;
sbit LCD_RS   = P1^2; 
sbit LCD_SDA  = P1^3; 
sbit LCD_SCL  = P1^4;

// --- 标准 6x8 ASCII 字库 (仅包含 HELLO 字符) ---
u8 code F6x8[][6] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 空格 [0]
    { 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00 }, // H [1]
    { 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00 }, // E [2]
    { 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00 }, // L [3]
    { 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00 }  // O [4]
};

void Delay_ms(u16 ms) {
    u16 i, j;
    for(i=ms; i>0; i--) for(j=114; j>0; j--);
}

void W_Byte(u8 dat, u8 is_data) {
    u8 i;
    LCD_CS = 0;
    LCD_RS = is_data; 
    for(i=0; i<8; i++) {
        LCD_SCL = 0;
        if(dat & 0x80) LCD_SDA = 1; else LCD_SDA = 0;
        _nop_(); _nop_();
        LCD_SCL = 1; 
        _nop_(); _nop_();
        dat <<= 1;
    }
    LCD_CS = 1;
}

void LcdSetPos(u8 page, u8 x) {
    W_Byte(0xB0 + page, 0); 
    W_Byte(0x10 + (x >> 4), 0); 
    W_Byte(x & 0x0F, 0);
}

void LcdClear() {
    u8 p, c;
    for(p=0; p<6; p++) {
        LcdSetPos(p, 0);
        for(c=0; c<96; c++) W_Byte(0x00, 1);
    }
}

// 显示一个 6x8 字符
void ShowChar(u8 page, u8 x, u8 index) {
    u8 i;
    LcdSetPos(page, x);
    for(i=0; i<6; i++) {
        W_Byte(F6x8[index][i], 1);
    }
}

void LcdInit() {
    LCD_RST = 0; Delay_ms(100); LCD_RST = 1; Delay_ms(100);
    W_Byte(0xE2, 0); Delay_ms(20);
    W_Byte(0xA2, 0); // 1/9 bias
    W_Byte(0xA0, 0); // ADC 正常
    W_Byte(0xC8, 0); // COM 翻转
    W_Byte(0x23, 0); // 粗调对比度
    W_Byte(0x81, 0); // 微调指令
    W_Byte(0x30, 0); // 亮度 (如果看不清调这个)
    W_Byte(0x2F, 0); // 电源开启
    W_Byte(0xAF, 0); // 显示开启
    LcdClear();
}

void main() {
    LcdInit();
    
    // 在屏幕中间显示 HELLO (Page 2, 从第 30 列开始)
    ShowChar(2, 30, 1); // H
    ShowChar(2, 36, 2); // E
    ShowChar(2, 42, 3); // L
    ShowChar(2, 48, 3); // L
    ShowChar(2, 54, 4); // O

    while(1);
}