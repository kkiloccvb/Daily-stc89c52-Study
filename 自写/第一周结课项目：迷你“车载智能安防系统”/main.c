#include <REGX52.H>

// --- A2开发板引脚校准 ---
sbit LED = P2^0;
sbit Buzzer = P2^5;
sbit Key1 = P3^1;
// 138译码器引脚（对应原理图P22, P23, P24）
sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;

// --- 全局变量 ---
unsigned char System_State = 0; // 0:设防(S1), 1:报警(AL)
unsigned char Seconds = 60;     
unsigned char T1_Count = 0;     
unsigned char PWM_Duty = 5;     
bit tick_10ms = 0;

// A2开发板原理图的共阴极段码表
// 0-9, A, L, S, -, 灭(0x00)
unsigned char NixieTable[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, // 0-9
    0x77, // A (索引10)
    0x38, // L (索引11)
    0x6D, // S (索引12)
    0x40  // - (索引13)
};

// 位选控制：确保顺序从左到右
// A2开发板通常 LED8 是最左边，LED1 是最右边
void Nixie_SetPos(unsigned char pos) {
    // 根据你的描述调整物理位置，如果显示还是反的，请将 pos 改为 (7-pos)
    LSA = pos & 0x01;
    LSB = (pos >> 1) & 0x01;
    LSC = (pos >> 2) & 0x01;
}

void Init_Timers() {
    TMOD = 0x11; 
    TH0 = 0xFC; TL0 = 0x66; // 1ms刷新
    TH1 = 0x3C; TL1 = 0xB0; // 50ms中断
    ET0 = 1; ET1 = 1; EA = 1; TR0 = 1; TR1 = 1;
}

void Timer0_Routine() interrupt 1 {
    static unsigned char scan_pos = 0;
    static unsigned char pwm_cnt = 0;
    static unsigned int alert_cnt = 0;
    TH0 = 0xFC; TL0 = 0x66; 

    // --- 硬件驱动逻辑 ---
    pwm_cnt++; if(pwm_cnt >= 100) pwm_cnt = 0;
    
    if(System_State == 0) { // 设防模式：呼吸感
        if(pwm_cnt < PWM_Duty) LED = 0; else LED = 1;
        Buzzer = 1; 
    } else { // 报警模式：同步闪烁
        alert_cnt++;
        if(alert_cnt < 100) { LED = 0; Buzzer = 0; }
        else if(alert_cnt < 200) { LED = 1; Buzzer = 1; }
        else alert_cnt = 0;
    }

    // --- 数码管显示逻辑 ---
    P0 = 0x00; // 消隐
    Nixie_SetPos(scan_pos);

    if(System_State == 0) { // 设防模式：显示 S1 
        if(scan_pos == 7) P0 = NixieTable[12]; // 最左位显示 S
        if(scan_pos == 6) P0 = NixieTable[1];  // 第二位显示 1
    } else { // 报警模式：显示 AL 和 倒计时
        if(scan_pos == 7) P0 = NixieTable[10]; // 最左位 A
        if(scan_pos == 6) P0 = NixieTable[11]; // 第二位 L
        // 倒计时显示在右侧
        if(scan_pos == 2) P0 = NixieTable[Seconds / 100]; 
        if(scan_pos == 1) P0 = NixieTable[(Seconds / 10) % 10];
        if(scan_pos == 0) P0 = NixieTable[Seconds % 10];
    }

    if(++scan_pos > 7) scan_pos = 0;
}

void Timer1_Routine() interrupt 3 {
    static unsigned char c1 = 0;
    TH1 = 0x3C; TL1 = 0xB0;
    if(++c1 >= 2) { tick_10ms = 1; c1 = 0; }
}

void main() {
    static bit last_key = 1;
    Init_Timers();
    while(1) {
        if(tick_10ms) {
            tick_10ms = 0;
            // 按键处理
            if(Key1 == 0 && last_key == 1) { 
                if(System_State == 0) { System_State = 1; Seconds = 60; }
                else { System_State = 0; }
            }
            last_key = Key1;

            // 计时逻辑
            if(System_State == 1) {
                T1_Count++;
                if(T1_Count >= 100) {
                    T1_Count = 0;
                    if(Seconds > 0) Seconds--;
                    else System_State = 0;
                }
            }
        }
    }
}