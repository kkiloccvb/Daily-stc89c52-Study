#include <REGX52.H>

sbit Buzzer = P2^5;        // 定义硬件：Buzzer 就是 P2.5 那个管脚
unsigned int Tone_Reload;  // 存核心变量：决定“音调”高低的数据
bit tick_flag = 0;         // 通信旗语：定时器 1 给 main 发的信号

// 1. 定时器0：负责产生声音频率（高频）
void Timer0_Init() {
    TMOD &= 0xF0; TMOD |= 0x01;
    TH0 = 0xF8; TL0 = 0x8C; // 初始设为 Do
    ET0 = 1; EA = 1; TR0 = 1;
}

// 2. 定时器1：负责产生报警节拍（低频，如 100ms）
void Timer1_Init() {
    TMOD &= 0x0F; TMOD |= 0x10;
    TH0 = 0x3C; TL0 = 0xB0; // 50ms
    ET1 = 1; TR1 = 1;
}

void Timer0_Routine() interrupt 1 {
    TH0 = Tone_Reload >> 8;   // 动态修改音调
    TL0 = Tone_Reload & 0xFF;
    Buzzer = ~Buzzer;         // 翻转电平产生方波
}

void Timer1_Routine() interrupt 3 {
    static unsigned char count = 0;
    TH1 = 0x3C; TL1 = 0xB0;
    count++;
    if(count >= 4) { // 200ms 响一次
        tick_flag = 1;
        count = 0;
    }
}

void main() {
    unsigned char alarm_mode = 0;
    Timer0_Init();
    Timer1_Init();
    Tone_Reload = 63628; // 初始频率

    while(1) {
        if(tick_flag == 1) { // 收到节拍信号
            tick_flag = 0;
            // 模拟无人车检测到距离变近，音调变高
            alarm_mode++;
            if(alarm_mode > 10) alarm_mode = 0;
            
            // 关键：动态调整频率
            Tone_Reload = 63628 + (alarm_mode * 200); 
        }
    }
}