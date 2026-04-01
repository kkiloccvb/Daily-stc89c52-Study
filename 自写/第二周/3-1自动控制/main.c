#include <REGX52.H>
#include <stdio.h>

// --- 引脚定义 ---
sbit AD_CS  = P3^5; 
sbit AD_CLK = P3^6;
sbit AD_DIN = P3^4;
sbit AD_DOUT= P3^7;
sbit DA_OUT = P2^1; // DAC输出（PWM模式）

// --- 全局变量 ---
unsigned char DA_Value = 0;    // 当前DAC输出值 (0-255)
unsigned char PWM_Count = 0; 
unsigned char Target_Value = 128; // 设定目标平衡点（约2.5V）
unsigned char Dead_Zone = 8;      // 工业死区：防止在目标点附近震荡

// --- 1. 串口初始化 (用于周三的逻辑监控) ---
void Uart_Init() {
    SCON = 0x50; TMOD &= 0x0F; TMOD |= 0x20;
    TH1 = 0xFD; TL1 = 0xFD; // 9600波特率 @11.0592MHz
    TR1 = 1; TI = 1;        // printf需要TI=1
}

// --- 2. 定时器0：高频PWM产生器 ---
void Timer0_Init() {
    TMOD &= 0xF0; TMOD |= 0x01; 
    TH0 = 0xFF; TL0 = 0xA4; // 100us节拍
    ET0 = 1; EA = 1; TR0 = 1;
}

void Timer0_Routine() interrupt 1 {
    TH0 = 0xFF; TL0 = 0xA4;
    PWM_Count++;
    if(PWM_Count < DA_Value) DA_OUT = 1; else DA_OUT = 0;
}

// --- 3. XPT2046 基础读取函数 ---
unsigned char Read_AD(unsigned char cmd) {
    unsigned char i, dat = 0;
    AD_CLK = 0; AD_CS = 0;
    for(i=0; i<8; i++) {
        AD_DIN = cmd & 0x80; cmd <<= 1;
        AD_CLK = 1; AD_CLK = 0;
    }
    for(i=0; i<8; i++) {
        AD_CLK = 1; dat <<= 1;
        if(AD_DOUT) dat |= 0x01;
        AD_CLK = 0;
    }
    AD_CS = 1;
    return dat;
}

// --- 4. 软件滤波：均值采样 ---
unsigned char Get_AD_Avg(unsigned char cmd) {
    unsigned int sum = 0;
    unsigned char i;
    for(i=0; i<8; i++) sum += Read_AD(cmd);
    return (unsigned char)(sum >> 3);
}

// --- 5. 主逻辑：闭环调节 ---
void main() {
    unsigned char current_val;
    Uart_Init();
    Timer0_Init();

    printf("System Start! Target: %d\n", (int)Target_Value);

    while(1) {
        current_val = Get_AD_Avg(0x94); // 读取电位器当前值

        // --- 核心：带死区的逻辑判断 ---
        // 如果当前值显著高于目标（超过死区上限）
        if(current_val > (Target_Value + Dead_Zone)) {
            if(DA_Value > 0) DA_Value--; // 降低输出，试图让输入降下来
        }
        // 如果当前值显著低于目标（低于死区下限）
        else if(current_val < (Target_Value - Dead_Zone)) {
            if(DA_Value < 255) DA_Value++; // 提高输出
        }
        // 否则（在死区内）：DA_Value 保持不变，系统静默，避免频繁动作

        // 每隔一段时间打印一次状态
        printf("Real:%bu | Target:%bu | PWM:%bu\r\n", current_val, Target_Value, DA_Value);

        // 这里的延时决定了控制系统的“阻尼”或“反应速度”
        // 工业上不能调太快，否则会产生超调
        {unsigned int i=10000; while(i--);} 
    }
}