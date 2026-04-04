#include <REGX52.H>
void Timer0_Init()
{
    TMOD &= 0xF0;       // 把 TMOD 的低 4 位清零（不影响定时器 1）
    TMOD |= 0x01;       // 设置定时器 0 为模式 1（16 位定时器）
    
    TH0 = 0x3C;         // 赋初值高 8 位 (15536 >> 8)
    TL0 = 0xB0;         // 赋初值低 8 位 (15536 & 0xFF)
    
    TF0 = 0;            // 清除溢出标志位
    ET0 = 1;            // 开启定时器 0 中断允许
    EA  = 1;            // 开启总中断（这一步最容易忘！）
    TR0 = 1;            // 定时器 0 开始运行
}

unsigned char count = 0; // 定义一个计数变量
unsigned char mode = 1;
static unsigned char last_key = 1;

void Timer0_Routine() interrupt 1
{
		unsigned char current_key;
    // 关键：必须手动重装初值，否则下次溢出就是从 0 开始，时间就不准了！
    TH0 = 0x3C; 
    TL0 = 0xB0;
		current_key = P3_1;
		if (last_key == 1 && current_key == 0) 
		{
			mode = !mode; // 翻转模式：快变慢，慢变快
		}
		last_key = current_key;
		
    count++;            // 每次进中断（50ms）计数加 1
    
    if(mode == 0 && count >= 20) // 慢速：1秒翻转
    {
        P2_0 = ~P2_0;
        count = 0;
    }
    if(mode == 1 && count >= 4)  // 快速：0.2秒翻转
    {
        P2_0 = ~P2_0;
        count = 0;
    }
}

void main()
{
    Timer0_Init();      // 执行初始化
    
    while(1)
    {
        // 这里可以写别的代码，灯的闪烁完全交给硬件中断处理
        // 这就是“非阻塞”编程的魅力
    }
}
