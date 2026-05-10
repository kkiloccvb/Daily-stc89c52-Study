#include <REGX52.H>
#include <INTRINS.H>

// --- 引脚定义 ---
sbit DS1302_SCLK = P3^6; 
sbit DS1302_IO   = P3^4; 
sbit DS1302_CE   = P3^5; 

sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;
#define LCD_DataPort P0

// 译码器 (关闭点阵/数码管)
sbit ADDR_A = P2^2;
sbit ADDR_B = P2^3;
sbit ADDR_C = P2^4;

// --- 延时 ---
void DelayMs(unsigned int ms) {
    unsigned char i, j;
    while(ms--) {
        i = 2; j = 239;
        do { while (--j); } while (--i);
    }
}

// --- LCD 驱动 ---
void Lcd_WriteCmd(unsigned char cmd) {
    LCD_RS=0; LCD_RW=0; LCD_DataPort=cmd;
    LCD_EN=1; _nop_(); LCD_EN=0; DelayMs(2);
}
void Lcd_WriteData(unsigned char dat) {
    LCD_RS=1; LCD_RW=0; LCD_DataPort=dat;
    LCD_EN=1; _nop_(); LCD_EN=0; DelayMs(2);
}
void Lcd_Init() {
    Lcd_WriteCmd(0x38); Lcd_WriteCmd(0x0C);
    Lcd_WriteCmd(0x06); Lcd_WriteCmd(0x01);
}

// --- DS1302 慢时序读写 ---
void DS1302_W(unsigned char dat) {
    unsigned char i;
    for(i=0; i<8; i++) {
        DS1302_IO = dat & 0x01;
        DS1302_SCLK = 1; _nop_(); _nop_(); // 增加脉冲宽度
        DS1302_SCLK = 0; _nop_(); _nop_();
        dat >>= 1;
    }
}
unsigned char DS1302_R() {
    unsigned char i, dat=0;
    for(i=0; i<8; i++) {
        dat >>= 1;
        DS1302_IO = 1; // 关键：释放总线
        if(DS1302_IO) dat |= 0x80;
        DS1302_SCLK = 1; _nop_(); _nop_();
        DS1302_SCLK = 0; _nop_(); _nop_();
    }
    return dat;
}

unsigned char Get_RTC(unsigned char addr) {
    unsigned char b;
    DS1302_CE=0; DS1302_SCLK=0; DS1302_CE=1;
    DS1302_W(addr | 0x01);
    b = DS1302_R();
    DS1302_CE=0;
    return (b/16)*10 + (b%16); // BCD转十进制
}

void Set_RTC(unsigned char addr, unsigned char dec) {
    unsigned char bcd = (dec/10)*16 + (dec%10);
    DS1302_CE=0; DS1302_SCLK=0; DS1302_CE=1;
    DS1302_W(addr);
    DS1302_W(bcd);
    DS1302_CE=0;
}

void main() {
    unsigned char s, m, h;
    
    // 1. 屏蔽点阵和数码管：这一步不跳过，否则P0口不稳
    ADDR_A=1; ADDR_B=1; ADDR_C=1; 
    
    Lcd_Init();
    
    // 2. 强制解锁并重置时钟
    DS1302_CE=1; DS1302_W(0x8E); DS1302_W(0x00); DS1302_CE=0;
    Set_RTC(0x80, 5);  // 秒 (CH位会被清零，强制起振)
    Set_RTC(0x82, 0);  // 分
    Set_RTC(0x84, 10); // 时

    while(1) {
        h = Get_RTC(0x84);
        m = Get_RTC(0x82);
        s = Get_RTC(0x80);

        // 第一行日期显示
        Lcd_WriteCmd(0x80);
        Lcd_WriteData('2'); Lcd_WriteData('0'); Lcd_WriteData('2'); Lcd_WriteData('6');
        Lcd_WriteData('-'); Lcd_WriteData('0'); Lcd_WriteData('4');
        Lcd_WriteData('-'); Lcd_WriteData('1'); Lcd_WriteData('5');

        // 第二行显示
        Lcd_WriteCmd(0x80 + 0x40);
        // 如果读出来全是0，说明RTC没工作，显示 E 代替
        if(h==0 && m==0 && s==0) {
            Lcd_WriteData('E'); Lcd_WriteData('r'); Lcd_WriteData('r');
        } else {
            Lcd_WriteData(h/10 + '0'); Lcd_WriteData(h%10 + '0');
            Lcd_WriteData(':');
            Lcd_WriteData(m/10 + '0'); Lcd_WriteData(m%10 + '0');
            Lcd_WriteData(':');
            Lcd_WriteData(s/10 + '0'); Lcd_WriteData(s%10 + '0');
        }
        
        DelayMs(200); 
    }
}