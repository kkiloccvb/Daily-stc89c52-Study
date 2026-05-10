#include <REGX52.H>

unsigned char T0Count = 0;  // 100步计数器 (0-99)
unsigned char Compare = 0;  // 亮度等级 (0, 20, 40, 60, 80, 100)
static unsigned char last_key = 1; // 记忆按键上一次状态
static unsigned int key_count = 0;
void Timer0_Init()
{
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = 0xFF; 
    TL0 = 0x9C; // 100微秒初值
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void Timer0_Routine() interrupt 1
{
    unsigned char current_key;
    TH0 = 0xFF; 
    TL0 = 0x9C;
    
    // --- 【1. PWM 核心逻辑】 ---
    T0Count++;
    if(T0Count >= 100) T0Count = 0;
    
    if(T0Count < Compare) P2_0 = 0; // 低电平点亮
    else P2_0 = 1;

    // --- 【2. 按键检测逻辑 (每100us扫一次)】 ---
    // 注意：这里有个测控小技巧，100us扫一次太快，会有细微抖动
    // 我们可以加一个计数器，每进 200 次中断（即20ms）才真正判一次键
    key_count++;
    if(key_count >= 200) 
    {
        key_count = 0;
        current_key = P3_1; // 读取按键
        
        if(last_key == 1 && current_key == 0) // 下降沿触发
        {
            Compare += 10; // 亮度加 20%
            if(Compare > 100) Compare = 0; // 循环回到最暗
        }
        last_key = current_key;
    }
}	

void main()
{
    Timer0_Init();
    while(1)
    {
        // CPU 现在完全空出来了！
        // 你可以在这里写显示数码管的代码，或者串口通信
    }
}