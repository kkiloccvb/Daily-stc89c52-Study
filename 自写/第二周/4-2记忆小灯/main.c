#include <REGX52.H>
#include <INTRINS.H>

sbit SCL = P2^1;
sbit SDA = P2^0; 
sbit K1 = P3^1;      
sbit K2 = P3^0;      

unsigned char Target_Value; 
unsigned char PWM_Count = 0;
unsigned char Device_Addr = 0xA0; // 默认地址

void I2C_Delay() { _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); }

void Delay(unsigned int ms) {
    unsigned char i, j;
    while (ms--) {
        i = 2; j = 239;
        do { while (--j); } while (--i);
    }
}

// --- 标准 I2C 驱动 ---
void I2C_Start() { SDA=1; SCL=1; I2C_Delay(); SDA=0; I2C_Delay(); SCL=0; }
void I2C_Stop()  { SDA=0; I2C_Delay(); SCL=1; I2C_Delay(); SDA=1; }

// 修改：增加应答检测返回值
bit I2C_Send(unsigned char Byte) {
    unsigned char i;
    bit ack;
    for(i=0; i<8; i++) {
        SDA = (Byte & (0x80 >> i));
        I2C_Delay(); SCL=1; I2C_Delay(); SCL=0;
    }
    SDA = 1; I2C_Delay(); SCL=1; 
    ack = SDA; // 读取芯片的应答信号（0为有应答）
    I2C_Delay(); SCL=0;
    return ack;
}

unsigned char I2C_Read() {
    unsigned char i, Byte = 0;
    SDA = 1; I2C_Delay();
    for(i=0; i<8; i++) {
        SCL = 1; I2C_Delay();
        if(SDA) Byte |= (0x80 >> i);
        SCL = 0; I2C_Delay();
    }
    SDA = 1; I2C_Delay(); SCL=1; I2C_Delay(); SCL=0;
    return Byte;
}

// --- 自动寻找芯片地址 ---
void Find_Device() {
    unsigned char addr;
    for(addr = 0xA0; addr < 0xAF; addr += 2) {
        I2C_Start();
        if(I2C_Send(addr) == 0) { // 发现应答
            Device_Addr = addr;
            I2C_Stop();
            return; 
        }
        I2C_Stop();
        Delay(1);
    }
}

void Save_To_EEPROM(unsigned char val) {
    TR0 = 0;
    I2C_Start();
    I2C_Send(Device_Addr); 
    I2C_Send(0x00); 
    I2C_Send(val);
    I2C_Stop();
    Delay(20);
    TR0 = 1;
}

unsigned char Load_From_EEPROM() {
    unsigned char val;
    I2C_Start();
    I2C_Send(Device_Addr); 
    I2C_Send(0x00); 
    I2C_Start();
    I2C_Send(Device_Addr | 0x01); 
    val = I2C_Read();
    I2C_Stop();
    return val;
}

void Timer0_Init() {
    TMOD &= 0xF0; TMOD |= 0x01;
    TH0 = 0xFF; TL0 = 0x9C;
    ET0 = 1; EA = 1; TR0 = 1;
}

void Timer0_Routine() interrupt 1 {
    TH0 = 0xFF; TL0 = 0x9C;
    PWM_Count++;
    if(PWM_Count < Target_Value) SDA = 0; else SDA = 1;
}

void main() {
    Find_Device(); // 先找人
    Target_Value = Load_From_EEPROM();
    
    if(Target_Value == 0xFF || Target_Value == 0) {
        // 闪 3 下：还是没找到人或读到空白
        unsigned char i;
        for(i=0; i<6; i++) { SDA = 0; Delay(200); SDA = 1; Delay(200); }
        Target_Value = 80;
        Save_To_EEPROM(Target_Value); 
    } else {
        // 闪 1 下：找到记忆了！
        SDA = 0; Delay(500); SDA = 1; Delay(200);
    }
    Timer0_Init();
    while(1) {
        if(K1 == 0) { Delay(20); if(K1 == 0) { 
            if(Target_Value <= 200) Target_Value += 50; 
            Save_To_EEPROM(Target_Value); while(K1 == 0); 
        }}
        if(K2 == 0) { Delay(20); if(K2 == 0) { 
            if(Target_Value >= 60) Target_Value -= 50; 
            Save_To_EEPROM(Target_Value); while(K2 == 0); 
        }}
    }
}