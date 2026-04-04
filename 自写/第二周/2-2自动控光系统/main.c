#include <REGX52.H>

// --- 引脚定义 (对照 A2 原理图) ---
sbit AD_CS  = P3^5; 
sbit AD_CLK = P3^6;
sbit AD_DIN = P3^4;
sbit AD_DOUT= P3^7;

sbit DA_OUT = P2^1; // DAC 模块输入引脚 (PWM输入口)

// --- 全局变量 ---
unsigned char DA_Value = 0;  // 目标占空比 (0-255)
unsigned char PWM_Count = 0; // PWM 累加计数器

// --- 1. 定时器0初始化：产生高频 PWM (约10kHz) ---
void Timer0_Init() {
    TMOD &= 0xF0; // 清高4位
    TMOD |= 0x01; // 16位定时器模式
    TH0 = 0xFF;   // 赋初值，中断频率要高，保证电压平滑
    TL0 = 0xA4; 
    ET0 = 1;      // 开启定时器0中断
    EA  = 1;      // 开启总中断
    TR0 = 1;      // 启动定时器
}

// --- 2. 定时器中断服务：实现 PWM 逻辑 ---
void Timer0_Routine() interrupt 1 {
    TH0 = 0xFF; 
    TL0 = 0xA4; 
    
    PWM_Count++; // 每次中断加1，到255后自动溢出回到0
    
    // 核心比较逻辑
    if(PWM_Count < DA_Value) {
        DA_OUT = 1; // 在占空比范围内，输出高电平
    } else {
        DA_OUT = 0; // 超出范围，输出低电平
    }
}

// --- 3. XPT2046 读取函数 (8位模式) ---
unsigned char Read_AD(unsigned char cmd) {
    unsigned char i, dat = 0;
    AD_CLK = 0;
    AD_CS  = 0; 

    // 发送命令
    for(i=0; i<8; i++) {
        AD_DIN = cmd & 0x80;
        cmd <<= 1;
        AD_CLK = 1; AD_CLK = 0; 
    }

    // 读取结果
    for(i=0; i<8; i++) {
        AD_CLK = 1;
        dat <<= 1;
        if(AD_DOUT) dat |= 0x01;
        AD_CLK = 0;
    }
    AD_CS = 1; 
    return dat;
}

// --- 4. 主程序 ---
void main() {
    Timer0_Init(); // 启动 PWM 引擎
    
    while(1) {
        // 读取 AIN0 通道 (电位器)
        // 将感应到的 0-255 数字量直接传给 DAC 输出变量
        DA_Value = 255-Read_AD(0x94); 
        
        // 适当延时，保证 ADC 采样电容稳定
        {unsigned int i=500; while(i--);}
    }
}