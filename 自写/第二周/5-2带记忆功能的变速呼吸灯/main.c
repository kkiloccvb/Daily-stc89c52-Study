#include <REGX52.H>
#include <INTRINS.H>

// --- 引脚定义 (使用你之前修正过的引脚) ---
sbit SCL = P2^1; 
sbit SDA = P2^0; 
sbit LED_D2 = P2^1; // PWM 输出给 D2
sbit K1 = P3^1;     // 加速
sbit K2 = P3^0;     // 减速

// --- 全局变量 ---
unsigned char Target_Value = 0;   // 当前亮度
unsigned char PWM_Count = 0;
unsigned int  Time_Tick = 0;      // 内部计数
unsigned char Breath_Speed = 20;  // 呼吸速度 (从 EEPROM 读取)
bit Direction = 0;                // 0:变亮, 1:变暗

// --- 基础延时 ---
void Delay(unsigned int ms) {
    unsigned char i, j;
    while (ms--) { i = 2; j = 239; do { while (--j); } while (--i); }
}

// --- I2C 驱动 (你之前调通的慢速版) ---
void I2C_Delay() { _nop_(); _nop_(); _nop_(); _nop_(); }
void I2C_Start() { SDA=1; SCL=1; I2C_Delay(); SDA=0; I2C_Delay(); SCL=0; }
void I2C_Stop()  { SDA=0; I2C_Delay(); SCL=1; I2C_Delay(); SDA=1; }
void I2C_Send(unsigned char Byte) {
    unsigned char i;
    for(i=0; i<8; i++) { SDA=(Byte&(0x80>>i)); I2C_Delay(); SCL=1; I2C_Delay(); SCL=0; }
    SDA=1; I2C_Delay(); SCL=1; I2C_Delay(); SCL=0;
}
unsigned char I2C_Read() {
    unsigned char i, Byte=0; SDA=1; I2C_Delay();
    for(i=0; i<8; i++){ SCL=1; I2C_Delay(); if(SDA) Byte|=(0x80>>i); SCL=0; I2C_Delay(); }
    SDA=1; I2C_Delay(); SCL=1; I2C_Delay(); SCL=0; return Byte;
}

// --- 存储逻辑 ---
void Save_Speed(unsigned char speed) {
    TR0 = 0; // 写期间停掉定时器，防止干扰
    I2C_Start(); I2C_Send(0xA0); I2C_Send(0x01); // 存在地址1，不影响之前的地址0
    I2C_Send(speed); I2C_Stop();
    Delay(15); TR0 = 1;
}

unsigned char Load_Speed() {
    unsigned char s;
    I2C_Start(); I2C_Send(0xA0); I2C_Send(0x01);
    I2C_Start(); I2C_Send(0xA1); s = I2C_Read(); I2C_Stop();
    return s;
}

// --- 定时器中断：同时控制 PWM 和 呼吸步进 ---
void Timer0_Init() {
    TMOD &= 0xF0; TMOD |= 0x01;
    TH0 = 0xFF; TL0 = 0x9C; // 100us
    ET0 = 1; EA = 1; TR0 = 1;
}

void Timer0_Routine() interrupt 1 {
    TH0 = 0xFF; TL0 = 0x9C;
    
    // 1. PWM 亮度
    PWM_Count++;
    if(PWM_Count < Target_Value) LED_D2 = 0; else LED_D2 = 1;

    // 2. 呼吸逻辑
    Time_Tick++;
    if(Time_Tick >= Breath_Speed) { // 根据速度变量来决定多久变一次亮度
        Time_Tick = 0;
        if(Direction == 0) {
            Target_Value++; if(Target_Value >= 250) Direction = 1;
        } else {
            Target_Value--; if(Target_Value <= 5) Direction = 0;
        }
    }
}

// --- 主程序 ---
void main() {
    // A. 开机读取上次的速度
    Breath_Speed = Load_Speed();
    if(Breath_Speed > 100 || Breath_Speed < 2) Breath_Speed = 20; // 默认速度
    
    Timer0_Init();

    while(1) {
        // K1: 加快呼吸 (数值越小，Time_Tick 达标越快)
        if(K1 == 0) {
            Delay(20);
            if(K1 == 0) {
                if(Breath_Speed > 5) Breath_Speed -= 5; 
                Save_Speed(Breath_Speed); // 记住新速度
                while(K1 == 0);
            }
        }
        
        // K2: 减慢呼吸 (数值越大，呼吸越稳)
        if(K2 == 0) {
            Delay(20);
            if(K2 == 0) {
                if(Breath_Speed < 95) Breath_Speed += 5;
                Save_Speed(Breath_Speed); // 记住新速度
                while(K2 == 0);
            }
        }
    }
}