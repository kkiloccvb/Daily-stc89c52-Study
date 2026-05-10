//#include <REGX52.H>

//// 根据 A2 开发板原理图确认引脚，是 P2^1, P2^0 
//sbit I2C_SCL = P2^0;
//sbit I2C_SDA = P2^1;

///**
//  * @brief  I2C起始信号
//  */
//void I2C_Start(void) {
//    I2C_SDA = 1;
//    I2C_SCL = 1; // 确保都在高位
//    I2C_SDA = 0; // SCL高时，SDA由高变低
//    I2C_SCL = 0;
//}

///**
//  * @brief  I2C停止信号
//  */
//void I2C_Stop(void) {
//    I2C_SDA = 0;
//    I2C_SCL = 1;
//    I2C_SDA = 1; // SCL高时，SDA由低变高
//}

///**
//  * @brief  I2C发送一个字节
//  */
//void I2C_SendByte(unsigned char Byte) {
//    unsigned char i;
//    for(i=0; i<8; i++) {
//        I2C_SDA = Byte & (0x80 >> i); // 从最高位开始放数据
//        I2C_SCL = 1; // 释放时钟，让对方读
//        I2C_SCL = 0; // 拉低时钟，准备放下一位
//    }
//}

///**
//  * @brief  I2C接收一个字节
//  */
//unsigned char I2C_ReceiveByte(void) {
//    unsigned char i, Byte = 0x00;
//    I2C_SDA = 1; // 释放总线，准备接收
//    for(i=0; i<8; i++) {
//        I2C_SCL = 1;
//        if(I2C_SDA) Byte |= (0x80 >> i);
//        I2C_SCL = 0;
//    }
//    return Byte;
//}

///**
//  * @brief  I2C接收应答 (ACK)
//  */
//void I2C_SendAck(unsigned char AckBit) {
//    I2C_SDA = AckBit;
//    I2C_SCL = 1;
//    I2C_SCL = 0;
//}

//#define AT24C02_ADDR 0xA0  // 芯片地址

///**
//  * @brief  向存储器写一个数据
//  * @param  WordAddress 要存的位置 (0-255)
//  * @param  Data 要存的数值
//  */
//void AT24C02_WriteByte(unsigned char WordAddress, unsigned char Data) {
//    I2C_Start();
//    I2C_SendByte(AT24C02_ADDR);      // 找芯片
//    I2C_SendByte(WordAddress);       // 找地址
//    I2C_SendByte(Data);              // 写数据
//    I2C_Stop();
//    // 关键！硬件写入需要时间，必须延时约 5ms
//    {unsigned int i=1000; while(i--);} 
//}

///**
//  * @brief  从存储器读一个数据
//  */
//unsigned char AT24C02_ReadByte(unsigned char WordAddress) {
//    unsigned char Data;
//    I2C_Start();
//    I2C_SendByte(AT24C02_ADDR);      // 找芯片（写模式）
//    I2C_SendByte(WordAddress);       // 找地址
//    I2C_Start();                     // 重启总线
//    I2C_SendByte(AT24C02_ADDR | 0x01); // 切换为读模式
//    Data = I2C_ReceiveByte();        // 拿数据
//    I2C_SendAck(1);                  // 发送非应答信号
//    I2C_Stop();
//    return Data;
//}
#include <REGX52.H>
#include <stdio.h>

// --- 引脚定义 ---
// ADC/DAC 引脚 (周三)
sbit AD_CS  = P3^5; 
sbit AD_CLK = P3^6;
sbit AD_DIN = P3^4;
sbit AD_DOUT= P3^7;
sbit DA_OUT = P2^1; 

// I2C/EEPROM 引脚 (周四)
sbit I2C_SCL = P2^0;
sbit I2C_SDA = P2^1;

// 按键引脚 (用于修改目标值)
sbit K1 = P3^1; // 加
sbit K2 = P3^0; // 减

// --- 全局变量 ---
unsigned char DA_Value = 0;    
unsigned char PWM_Count = 0; 
unsigned char Target_Value;     // 目标值（将从EEPROM读取）
unsigned char Dead_Zone = 8;    

// ================== I2C & AT24C02 驱动 (存储) ==================

void I2C_Start() { I2C_SDA=1; I2C_SCL=1; I2C_SDA=0; I2C_SCL=0; }
void I2C_Stop()  { I2C_SDA=0; I2C_SCL=1; I2C_SDA=1; }
void I2C_SendByte(unsigned char Byte) {
    unsigned char i;
    for(i=0; i<8; i++) { I2C_SDA=Byte&(0x80>>i); I2C_SCL=1; I2C_SCL=0; }
    I2C_SCL=1; I2C_SCL=0; // 简化的应答处理
}
unsigned char I2C_ReceiveByte() {
    unsigned char i, Byte=0; I2C_SDA=1;
    for(i=0; i<8; i++) { I2C_SCL=1; if(I2C_SDA) Byte|=(0x80>>i); I2C_SCL=0; }
    return Byte;
}

void Save_Target(unsigned char val) {
    I2C_Start(); I2C_SendByte(0xA0); I2C_SendByte(0x01); // 存在地址0x01
    I2C_SendByte(val); I2C_Stop();
    {unsigned int i=1000; while(i--);} // 等待5ms写入完成
}

unsigned char Load_Target() {
    unsigned char val;
    I2C_Start(); I2C_SendByte(0xA0); I2C_SendByte(0x01);
    I2C_Start(); I2C_SendByte(0xA1); val=I2C_ReceiveByte();
    I2C_SCL=1; I2C_SCL=0; I2C_Stop();
    return val;
}

// ================== ADC & PWM 驱动 (感知与执行) ==================

unsigned char Read_AD(unsigned char cmd) {
    unsigned char i, dat=0; AD_CLK=0; AD_CS=0;
    for(i=0; i<8; i++) { AD_DIN=cmd&0x80; cmd<<=1; AD_CLK=1; AD_CLK=0; }
    for(i=0; i<8; i++) { AD_CLK=1; dat<<=1; if(AD_DOUT) dat|=1; AD_CLK=0; }
    AD_CS=1; return dat;
}

void Timer0_Init() { TMOD|=0x01; TH0=0xFF; TL0=0xA4; ET0=1; EA=1; TR0=1; }
void Timer0_Routine() interrupt 1 {
    TH0=0xFF; TL0=0xA4; PWM_Count++;
    if(PWM_Count < DA_Value) DA_OUT=1; else DA_OUT=0;
}

// ================== 主程序 (系统逻辑) ==================

void main() {
    unsigned char current_val;
    Timer0_Init();
    
    // 【周四新增】：开机第一件事，读取记忆
    Target_Value = Load_Target();
    if(Target_Value == 0xFF || Target_Value == 0) Target_Value = 128; // 第一次运行赋予默认值

    while(1) {
        current_val = Read_AD(0x94); // 获取当前值

        // 【周四新增】：按键修改目标并保存
        if(K1 == 0) { // 按下K1增加目标
            while(K1==0); // 消抖
            if(Target_Value < 240) Target_Value += 10;
            Save_Target(Target_Value); // 存入“硬盘”
        }
        if(K2 == 0) { // 按下K2减少目标
            while(K2==0); 
            if(Target_Value > 10) Target_Value -= 10;
            Save_Target(Target_Value); // 存入“硬盘”
        }

        // 【周三核心】：闭环判断
        if(current_val > (Target_Value + Dead_Zone)) {
            if(DA_Value > 0) DA_Value--; 
        }
        else if(current_val < (Target_Value - Dead_Zone)) {
            if(DA_Value < 255) DA_Value++;
        }

        // 节拍控制
        {unsigned int i=5000; while(i--);} 
    }
}