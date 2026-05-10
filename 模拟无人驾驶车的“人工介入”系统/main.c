#include <REGX52.H>

// --- 引脚定义 ---
sbit LED = P2^0;
sbit Buzzer = P2^5;
sbit Key1 = P3^1;

// --- 全局变量（标志位与数据） ---
static unsigned int key_count = 0;
unsigned int Tone_Reload = 63628; 
unsigned char PWM_Duty = 0;      // LED 亮度占空比
bit tick_200ms = 0;             // 报警节奏标志
bit tick_10ms = 0;              // 亮度更新标志
bit Emergency_Mode = 0;         // 紧急制动模式标志位

// --- 定时器初始化 ---
void Init_Timers() {
    TMOD = 0x11; // 定时器0和1都用模式1 (16位)
    // T0 负责音调和PWM计数 (约100us一次)
    TH0 = 0xFF; TL0 = 0x9C; 
    // T1 负责逻辑节拍 (50ms一次)
    TH1 = 0x3C; TL1 = 0xB0;
    ET0 = 1; ET1 = 1; EA = 1; TR0 = 1; TR1 = 1;
}

// --- 后台：高频动作 (声音+光) ---
void Timer0_Routine() interrupt 1 {
    static unsigned char pwm_count = 0;
    TH0 = Tone_Reload >> 8;
    TL0 = Tone_Reload & 0xFF;

    if(!Emergency_Mode) { // 正常模式：产生报警方波
        Buzzer = ~Buzzer;
        // PWM 控制 LED 亮度
        pwm_count++;
        if(pwm_count >= 100) pwm_count = 0;
        if(pwm_count < PWM_Duty) LED = 0; else LED = 1;
    } else { // 紧急模式：蜂鸣器长鸣，灯常亮
        Buzzer = 0; 
        LED = 0;
    }
}

// --- 后台：节奏管理 ---
void Timer1_Routine() interrupt 3 {
    static unsigned char c1 = 0, c2 = 0;
    TH1 = 0x3C; TL1 = 0xB0;
    c1++; c2++;
    if(c1 >= 2) { tick_10ms = 1; c1 = 0; }
    if(c2 >= 4) { tick_200ms = 1; c2 = 0; }
}

// --- 前台：决策逻辑 ---
void main() {
    signed char dir = 1;
    Init_Timers();
    while(1) {
        // 1. 处理亮度变化 (每10ms)
        if(tick_10ms && !Emergency_Mode) {
            tick_10ms = 0;
            PWM_Duty += dir;
            if(PWM_Duty >= 100 || PWM_Duty <= 0) dir = -dir;
        }
        // 2. 处理音调变化 (每200ms)
        if(tick_200ms && !Emergency_Mode) {
            tick_200ms = 0;
            Tone_Reload += 100; // 模拟距离缩短，音调逐渐变尖
            if(Tone_Reload > 65000) Tone_Reload = 63628;
        }
        // 3. 按键检测 (人工介入)
				// 在 main 的 while(1) 里面修改第三部分：
				// 3. 按键检测 (人工介入) - 状态机/边沿检测法
				if(tick_10ms) { // 借用 10ms 节拍作为消抖周期
						static bit last_key_state = 1; // 记录上一次按键状态
						if(Key1 == 0 && last_key_state == 1) { // 捕捉下降沿：之前是高，现在是低
								Emergency_Mode = !Emergency_Mode;  // 翻转模式
						}
						last_key_state = Key1; // 更新状态记忆
					}
			}
    }
