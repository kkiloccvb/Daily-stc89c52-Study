#include <REGX52.H>

// --- 引脚定义 ---
sbit LED = P2^0;
sbit Buzzer = P2^5;
sbit Key1 = P3^1;
// 138译码器引脚
sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;

// --- 全局变量 ---
unsigned int Tone_Reload = 63628; 
unsigned char PWM_Duty = 5;      
bit tick_200ms = 0;             
bit tick_10ms = 0;              
bit Emergency_Mode = 0;         

// 数码管段码表
unsigned char NixieTable[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

// 位选控制函数 (对应你发的那张 74HC138 原理图)
void Nixie_SetPos(unsigned char pos) {
    pos--; // 1-8 映射到 0-7
    LSA = pos & 0x01;
    LSB = (pos >> 1) & 0x01;
    LSC = (pos >> 2) & 0x01;
}

void Init_Timers() {
    TMOD = 0x11; 
    TH0 = 0xFC; TL0 = 0x66; // 约 1ms 刷新一次
    TH1 = 0x3C; TL1 = 0xB0; 
    ET0 = 1; ET1 = 1; EA = 1; TR0 = 1; TR1 = 1;
}

// 【核心：多任务合一的定时器0】
void Timer0_Routine() interrupt 1 {
    static unsigned char pwm_count = 0;
    static unsigned char scan_pos = 1;
    static unsigned int buzzer_count = 0;

    // 1. 重新装载 (1ms刷新)
    TH0 = 0xFC; TL0 = 0x66;

    // 2. 声音逻辑
    if(!Emergency_Mode) {
        // 模拟方波：在1ms的中断里，我们需要更细的计数来产生千赫兹的声音
        // 这里简化：让它随中断翻转
        buzzer_count++;
        if(buzzer_count >= 2) { // 约 500Hz
            Buzzer = ~Buzzer;
            buzzer_count = 0;
        }
    } else {
        Buzzer = 0; // 紧急模式常鸣（取决于硬件，有的0响有的1响）
    }

    // 3. PWM 灯光逻辑
    pwm_count++;
    if(pwm_count >= 100) pwm_count = 0;
    if(!Emergency_Mode) {
        if(pwm_count < PWM_Duty) LED = 0; else LED = 1;
    } else {
        LED = 0; // 紧急模式常亮
    }

    // 4. 数码管刷新逻辑 (扫描)
    P0 = 0x00; // 消隐
    Nixie_SetPos(scan_pos);
    if(scan_pos == 1) P0 = NixieTable[PWM_Duty / 10]; // 亮度十位
    if(scan_pos == 2) P0 = NixieTable[PWM_Duty % 10]; // 亮度个位
    if(scan_pos == 8) P0 = NixieTable[Emergency_Mode]; // 最后一位显示模式(0/1)

    scan_pos++;
    if(scan_pos > 8) scan_pos = 1;
}

void Timer1_Routine() interrupt 3 {
    static unsigned char c1 = 0, c2 = 0;
    TH1 = 0x3C; TL1 = 0xB0;
    c1++; c2++;
    if(c1 >= 2) { tick_10ms = 1; c1 = 0; }
    if(c2 >= 4) { tick_200ms = 1; c2 = 0; }
}

void main() {
    signed char dir = 1;
    static bit last_key_state = 1; 
    Init_Timers();
    while(1) {
        // 决策逻辑不变...
        if(tick_10ms && !Emergency_Mode) {
            tick_10ms = 0;
            PWM_Duty += dir;
            if(PWM_Duty >= 100 || PWM_Duty <= 0) dir = -dir;
        }
        if(tick_200ms && !Emergency_Mode) {
            tick_200ms = 0;
            // 可以在这里改变显示频率等逻辑
        }
        // 按键边沿检测
        if(tick_10ms) {
            if(Key1 == 0 && last_key_state == 1) {
                Emergency_Mode = !Emergency_Mode; 
            }
            last_key_state = Key1;
        }
    }
}