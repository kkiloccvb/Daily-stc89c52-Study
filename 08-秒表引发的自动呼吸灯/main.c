#include <REGX52.H>

sbit LED_D2 = P2^1; 
unsigned char Target_Value = 0; // 当前亮度
unsigned char PWM_Count = 0;
unsigned int Time_Count = 0;    // 用来控制呼吸速度的计数器
bit Direction = 0;              // 呼吸方向：0为变亮，1为变暗

void Timer0_Init() {
    TMOD &= 0xF0; TMOD |= 0x01;
    TH0 = 0xFF; TL0 = 0x9C; // 100微秒中断一次
    ET0 = 1; EA = 1; TR0 = 1;
}

void Timer0_Routine() interrupt 1 {
    TH0 = 0xFF; TL0 = 0x9C;
    
    // --- 部分 A：PWM 亮度控制 (周三/周四的功底) ---
    PWM_Count++;
    if(PWM_Count < Target_Value) LED_D2 = 0; 
    else LED_D2 = 1;

    // --- 部分 B：自动呼吸算法 (周五的新内容) ---
    Time_Count++;
    if(Time_Count >= 50) { // 每 2ms (50 * 100us) 改变一次亮度
        Time_Count = 0;
        
        if(Direction == 0) {    // 变亮过程
            Target_Value++;
            if(Target_Value >= 250) Direction = 1; // 到达顶峰，开始变暗
        }
        else {                  // 变暗过程
            Target_Value--;
            if(Target_Value <= 5) Direction = 0;   // 到达谷底，开始变亮
        }
    }
}

void main() {
    // 这里可以保留你周四的 Load_From_EEPROM() 
    // 让呼吸灯从你上次设定的那个亮度开始“起吸”
    
    Timer0_Init();
    while(1) {
        // 周五的任务通常不需要在 while(1) 里写太多
        // 因为一切都在中断（后台）里自动完成了
    }
}