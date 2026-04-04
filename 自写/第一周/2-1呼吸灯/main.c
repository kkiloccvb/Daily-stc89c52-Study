#include <REGX52.H>

unsigned char T0Count = 0; 
unsigned char Compare = 0; 
unsigned char Dir = 0;     
bit tick_flag = 0;         // 信号旗

void Timer0_Routine() interrupt 1
{
    static unsigned char loop_count = 0; // 专门的计数器
    TH0 = 0xFF; TL0 = 0x9C; // 100us

    // --- 后台任务 1：高频 PWM 控灯 (每 100us 一次) ---
    T0Count++;
    if(T0Count >= 100) T0Count = 0;
    if(T0Count < Compare) P2_0 = 0; else P2_0 = 1;

    // --- 后台任务 2：定时发信 (每 10ms 发一次信) ---
    if(T0Count == 0) // 每一轮 PWM 结束时
    {
        loop_count++;
        if(loop_count >= 5) // 如果想更慢，把 5 改大，比如 50
        {
            tick_flag = 1;  // 举旗子！告诉 main 该干活了
            loop_count = 0;
        }
    }
}

void main()
{
    Timer0_Init();
    while(1)
    {
        // --- 前台任务：响应信号，处理逻辑 ---
        if(tick_flag == 1) 
        {
            tick_flag = 0; // 先放下旗子，防止重复处理

            if(Dir == 0) {
                Compare++;
                if(Compare >= 100) Dir = 1;
            } else {
                Compare--;
                if(Compare <= 0) Dir = 0;
            }
        }
    }
}