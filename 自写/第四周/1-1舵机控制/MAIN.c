#include <REGX52.H>

unsigned char Counter = 0;      // 计数器
unsigned char DutyCycle = 30;   // 占空比（0-100），现在是30%

sbit PWM_Out = P2^1;            // 假设马达驱动或LED接在P2.1

// 定时器0初始化
void Timer0Init() {
    TMOD &= 0xF0;       // 设置定时器模式
    TMOD |= 0x01;       // 16位定时器模式
    TL0 = 0xA4;         // 设置定时初值（11.0592MHz下，约100us）
    TH0 = 0xFF;         
    TF0 = 0;            // 清除TF0标志
    TR0 = 1;            // 定时器0开始计时
    ET0 = 1;            // 开启定时器0中断
    EA = 1;             // 开启总中断
}

void main() {
    Timer0Init();
    while(1) {
        // 这里可以写你的红外解码逻辑
        // 比如：如果按键1，DutyCycle = 30;
        // 比如：如果按键2，DutyCycle = 60;
    }
}

// 定时器0中断服务程序
void Timer0_Routine() interrupt 1 {
    TL0 = 0xA4;         // 重新赋初值
    TH0 = 0xFF;
    
    Counter++;          // 每次中断+1
    if(Counter >= 100) {
        Counter = 0;    // 满100清零，完成一个周期
    }
    
    if(Counter < DutyCycle) {
        PWM_Out = 1;    // 在占空比时间内，输出高
    } else {
        PWM_Out = 0;    // 超过占空比时间，输出低
    }
}