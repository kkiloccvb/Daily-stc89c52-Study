#include <REGX52.H>

// --- 引脚定义 (根据 A2 原理图) ---
sbit AD_CS  = P3^5; 
sbit AD_CLK = P3^6;
sbit AD_DIN = P3^4;
sbit AD_DOUT= P3^7;

// --- 串口初始化 (4800bps) ---
void Uart_Init() {
    SCON = 0x50; PCON |= 0x80;
    TMOD &= 0x0F; TMOD |= 0x20;
    TL1 = 0xF4; TH1 = 0xF4;
    TR1 = 1;
}

void Uart_SendByte(unsigned char Byte) {
    SBUF = Byte;
    while(TI == 0);
    TI = 0;
}

void Uart_SendString(char *str) {
    while (*str != '\0') Uart_SendByte(*str++);
}

// --- ADC 核心函数 ---
unsigned char Read_AD(unsigned char cmd) {
    unsigned char i, dat = 0;
    AD_CS = 0;  // 开始通信
    AD_CLK = 0;
    
    // 写入命令
    for(i=0; i<8; i++) {
        AD_DIN = cmd & 0x80;
        cmd <<= 1;
        AD_CLK = 1; AD_CLK = 0; // 产生脉冲
    }
    
    // 芯片处理需要一点点微小的时间
    for(i=0; i<6; i++); 

    // 读取 8 位结果 (注意：XPT2046 实际上是 12 位，我们取高 8 位方便处理)
    for(i=0; i<8; i++) {
        AD_CLK = 1;
        dat <<= 1;
        if(AD_DOUT) dat |= 0x01;
        AD_CLK = 0;
    }
    AD_CS = 1; // 结束通信
    return dat;
}

void main() {
    unsigned char val;
    Uart_Init();
    
    Uart_SendString("ADC Test Start...\r\n");
    
    while(1) {
        val = Read_AD(0xD4); // 0x94 对应旋钮电位器 0xA4光敏电阻 0xD4热敏电阻
        
        Uart_SendString("Potentiometer Value: ");
        Uart_SendByte(val / 100 + '0');    // 百位
        Uart_SendByte((val / 10) % 10 + '0'); // 十位
        Uart_SendByte(val % 10 + '0');    // 个位
        Uart_SendString("\r\n");

        // 延时 500ms 左右，防止串口刷屏太快
        {unsigned int i=50000; while(i--); i=50000; while(i--);}
    }
}