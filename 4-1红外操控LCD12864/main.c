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
sbit IRIN     = P3^2; 
sbit LED      = P2^0; 

// --- 全局变量 ---
u8 IrValue[4]; 
u8 IrOK = 0;
u8 code Hex_Table[] = "0123456789ABCDEF";

// 你的遥控器键值映射表 (0-9 对应你测出的乱码)
u8 code Key_Map[] = {
    0x16, // 0
    0x0C, // 1
    0x18, // 2
    0x5E, // 3
    0x08, // 4
    0x1C, // 5
    0x5A, // 6
    0x42, // 7
    0x52, // 8
    0x4A  // 9
};

// --- 6x8 字库 ---
u8 code F6x8[][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, // [0] 空格
    {0x7F,0x08,0x14,0x22,0x41,0x00}, // [1] K
    {0x7F,0x49,0x49,0x49,0x41,0x00}, // [2] E
    {0x03,0x04,0x78,0x04,0x03,0x00}, // [3] Y
    {0x00,0x36,0x36,0x00,0x00,0x00}, // [4] :
    {0x3E,0x51,0x49,0x45,0x3E,0x00}, // [5] 0
    {0x00,0x42,0x7F,0x40,0x00,0x00}, // [6] 1
    {0x42,0x61,0x51,0x49,0x46,0x00}, // [7] 2
    {0x21,0x41,0x45,0x4B,0x31,0x00}, // [8] 3
    {0x18,0x14,0x12,0x7F,0x10,0x00}, // [9] 4
    {0x27,0x45,0x45,0x45,0x39,0x00}, // [10] 5
    {0x3C,0x4A,0x49,0x49,0x30,0x00}, // [11] 6
    {0x01,0x71,0x09,0x05,0x03,0x00}, // [12] 7
    {0x36,0x49,0x49,0x49,0x36,0x00}, // [13] 8
    {0x06,0x49,0x49,0x29,0x1E,0x00}, // [14] 9
    {0x7C,0x12,0x11,0x12,0x7C,0x00}, // [15] A
    {0x7F,0x49,0x49,0x49,0x36,0x00}, // [16] B
    {0x4F,0x41,0x41,0x41,0x31,0x00}, // [17] M (简单画法)
    {0x7F,0x41,0x41,0x22,0x1C,0x00}  // [18] D
};

// --- 基础驱动 ---
void Delay_ms(u16 ms) { u16 i, j; for(i=ms; i>0; i--) for(j=114; j>0; j--); }
void W_Byte(u8 dat, u8 is_data) {
    u8 i; LCD_CS = 0; LCD_RS = is_data; 
    for(i=0; i<8; i++) { LCD_SCL = 0; LCD_SDA = dat & 0x80; LCD_SCL = 1; dat <<= 1; }
    LCD_CS = 1;
}
void LcdSetPos(u8 p, u8 x) { W_Byte(0xB0 + p, 0); W_Byte(0x10 + (x >> 4), 0); W_Byte(x & 0x0F, 0); }
void ShowChar(u8 p, u8 x, u8 ch) {
    u8 i, id = 0;
    if(ch=='K') id=1; else if(ch=='E') id=2; else if(ch=='Y') id=3; else if(ch==':') id=4;
    else if(ch>='0'&&ch<='9') id=ch-'0'+5; else if(ch=='M') id=17; else if(ch=='D') id=18;
    LcdSetPos(p, x); for(i=0; i<6; i++) W_Byte(F6x8[id][i], 1);
}

void LcdInit() {
    LCD_RST=0; Delay_ms(50); LCD_RST=1; Delay_ms(50);
    W_Byte(0xE2,0); W_Byte(0xA2,0); W_Byte(0xA0,0); W_Byte(0xC8,0);
    W_Byte(0x23,0); W_Byte(0x81,0); W_Byte(0x32,0); W_Byte(0x2F,0); W_Byte(0xAF,0);
    { u8 i, j; for(i=0;i<6;i++){ LcdSetPos(i,0); for(j=0;j<96;j++) W_Byte(0,1); } }
}

// --- 中断解码 ---
void ReadIr() interrupt 0 {
    u8 i, j, time; u16 timeout;
    timeout = 0; while(IRIN == 0 && timeout < 1000) { _nop_(); timeout++; } 
    if(timeout < 200) return;
    timeout = 0; while(IRIN == 1 && timeout < 500) { _nop_(); timeout++; }
    for(i=0; i<4; i++) {
        for(j=0; j<8; j++) {
            timeout = 0; while(IRIN == 0 && timeout < 100) { _nop_(); timeout++; }
            time = 0; while(IRIN == 1 && time < 100) { { u8 t=10; while(t--); } time++; }
            IrValue[i] >>= 1; if(time > 7) IrValue[i] |= 0x80;
        }
    }
    if(IrValue[2] == (u8)~IrValue[3]) IrOK = 1;
}

// --- 主程序 ---
void main() {
    LcdInit();
    IT0 = 1; EX0 = 1; EA = 1; IRIN = 1;

    ShowChar(2, 10, 'K'); ShowChar(2, 16, 'E'); 
    ShowChar(2, 22, 'Y'); ShowChar(2, 28, ':');

    while(1) {
        LED = !LED; 
        Delay_ms(200);

        if(IrOK) {
            u8 i;
            u8 found = 0;

            // 1. 查找 0-9 映射
            for(i=0; i<10; i++) {
                if(IrValue[2] == Key_Map[i]) {
                    ShowChar(2, 40, i + '0'); // 显示翻译后的数字
                    ShowChar(2, 46, ' ');     // 擦除旧字符
                    found = 1;
                    break;
                }
            }

            // 2. 如果没找到，判断是否是特殊键
            if(!found) {
                if(IrValue[2] == 0x46) { // MODE键
                    ShowChar(2, 40, 'M'); 
                    ShowChar(2, 46, 'D');
                } else {
                    // 既不是数字也不是MODE，显示原始十六进制方便调试
                    ShowChar(2, 40, Hex_Table[IrValue[2] >> 4]);
                    ShowChar(2, 46, Hex_Table[IrValue[2] & 0x0F]);
                }
            }
            IrOK = 0;
        }
    }
}