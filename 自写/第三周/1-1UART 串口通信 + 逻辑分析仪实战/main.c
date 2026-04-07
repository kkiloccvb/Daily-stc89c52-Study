#include <REGX52.H>
#include <stdio.h>

sbit LED_D2 = P2^1;
unsigned char Breath_Speed = 20;
unsigned char PWM_Count = 0;
unsigned int  Time_Tick = 0;
unsigned char Target_Value = 0;
bit Direction = 0;

sbit K3 = P3^2; // 对应开发板上的 K3
sbit K4 = P3^3; // 对应开发板上的 K4

// --- 必须添加这个延时函数 ---
void Delay(unsigned int ms) {
    unsigned char i, j;
    while (ms--) {
        i = 2; j = 239;
        do { while (--j); } while (--i);
    }
}

// --- 串口初始化 ---
void UART_Init() {
    SCON = 0x50;
    TMOD &= 0x0F; TMOD |= 0x20; // 定时器1模式2
    TH1 = 0xFD; TL1 = 0xFD;     // 9600波特率 @ 11.0592MHz
    TR1 = 1; 
    TI = 1;                     // printf 必点火
}

void UART_SendByte(unsigned char Byte) {
    SBUF = Byte;
    while(TI == 0);
    TI = 0;
}

char putchar(char c) {
    UART_SendByte(c);
    return c;
}

// --- 定时器0：控制 PWM 和呼吸节奏 ---
void Timer0_Init() {
    TMOD &= 0xF0; TMOD |= 0x01; // 定时器0模式1
    TH0 = 0xFF; TL0 = 0x9C;     // 100us
    ET0 = 1; EA = 1; TR0 = 1;
}

void Timer0_Routine() interrupt 1 {
    TH0 = 0xFF; TL0 = 0x9C;
    PWM_Count++;
    if(PWM_Count < Target_Value) LED_D2 = 0; else LED_D2 = 1;

    Time_Tick++;
    if(Time_Tick >= Breath_Speed) {
        Time_Tick = 0;
        if(Direction == 0) {
            Target_Value++; if(Target_Value >= 250) Direction = 1;
        } else {
            Target_Value--; if(Target_Value <= 5) Direction = 0;
        }
    }
}

void main() {
    UART_Init();
    Timer0_Init();
    
    printf("System Start...\r\n");
    
    while(1) {
        // K3 按下：加速 (Breath_Speed 减小)
        if(K3 == 0) {  
            Delay(20); // 防抖
            if(K3 == 0) {
                if(Breath_Speed > 2) Breath_Speed -= 2;
                printf("Speed Up! Current:%d\r\n", (int)Breath_Speed);
                while(K3 == 0); // 死循环直到松手
            }
        }
        
        // K4 按下：减速 (Breath_Speed 增大)
        if(K4 == 0) {  
            Delay(20); 
            if(K4 == 0) {
                if(Breath_Speed < 250) Breath_Speed += 2;
                printf("Speed Down! Current:%d\r\n", (int)Breath_Speed);
                while(K4 == 0); 
            }
        }
    }
}