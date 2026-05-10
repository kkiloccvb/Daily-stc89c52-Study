#include <REGX52.H>

// --- 1. 硬件引脚定义 (对应 A2 原理图) ---
sbit LED = P2^0;
sbit Buzzer = P2^5;
sbit Key1 = P3^1; // 注意：P3.1 也是 TXD，按键时串口可能会有轻微干扰
sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;

// --- 2. 全局变量 (放在所有函数外面，大家都能用) ---
unsigned char System_State = 0; 
unsigned char Seconds = 60;     
unsigned char T1_Count = 0;     
bit tick_10ms = 0;              
unsigned char NixieTable[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x38}; // 0-9, A, L

// --- 3. 串口相关函数 ---
void Uart_Init() {
    SCON = 0x50; PCON |= 0x80;
    TMOD &= 0x0F; TMOD |= 0x20; // T1 方式2：8位自动重装
    TL1 = 0xF4; TH1 = 0xF4;     // 4800bps @ 11.0592MHz
    ET1 = 0; TR1 = 1; 
}

void Uart_SendByte(unsigned char Byte) {
    SBUF = Byte;
    while(TI == 0);
    TI = 0;
}

void Uart_SendString(char *str) {
    while (*str != '\0') Uart_SendByte(*str++);
}

// --- 4. 数码管与定时器初始化 ---
void Nixie_SetPos(unsigned char pos) {
    LSA = pos & 0x01; LSB = (pos >> 1) & 0x01; LSC = (pos >> 2) & 0x01;
}

void Init_Timers() {
    // TMOD 已经在 Uart_Init 里设置过高4位了，这里只动低4位
    TMOD &= 0xF0; TMOD |= 0x01; // T0 方式1：16位定时器
    TH0 = 0xFC; TL0 = 0x66;     // 1ms
    ET0 = 1; EA = 1; TR0 = 1;
}

// --- 5. 中断服务函数 (系统的“后台”任务) ---
void Timer0_Routine() interrupt 1 {
    static unsigned char scan_pos = 0;
    static unsigned int alert_cnt = 0;
    TH0 = 0xFC; TL0 = 0x66;

    // 数码管扫描
    P0 = 0x00; 
    Nixie_SetPos(scan_pos);
    if(System_State == 0) {
        if(scan_pos == 7) P0 = 0x6D; // 'S'
        if(scan_pos == 6) P0 = 0x06; // '1'
    } else {
        if(scan_pos == 7) P0 = 0x77; // 'A'
        if(scan_pos == 6) P0 = 0x38; // 'L'
        if(scan_pos == 1) P0 = NixieTable[Seconds / 10];
        if(scan_pos == 0) P0 = NixieTable[Seconds % 10];
    }
    if(++scan_pos > 7) scan_pos = 0;

    // 报警声光逻辑
    if(System_State == 1) {
        alert_cnt++;
        if(alert_cnt < 100) { LED = 0; Buzzer = 0; }
        else if(alert_cnt < 200) { LED = 1; Buzzer = 1; }
        else alert_cnt = 0;
    } else { LED = 1; Buzzer = 1; }
}

// 利用 T0 软件计数产生 10ms 节拍
// (这里为了不占用 T1，我们把 10ms 逻辑也放进 T0 或者 main)

// --- 6. 主程序 (系统的“前台”决策) ---
void main() {
    static bit last_key = 1;
    static unsigned int debounce_cnt = 0; // 消抖计数
    
    Uart_Init();   
    Init_Timers(); 
    
    Uart_SendString("Hachimi System Online!\r\n");

    while(1) {
        // --- 核心：软件消抖逻辑 ---
        // 只有当程序运行了一段时间（模拟20ms），我们才去扫一次按键
        debounce_cnt++;
        if(debounce_cnt >= 8000) { // 调整这个数值可以改变灵敏度
            debounce_cnt = 0;
            
            // 1. 捕捉按下 (下降沿)
            if (Key1 == 0 && last_key == 1) { 
                System_State = !System_State;
                if(System_State == 1) Seconds = 60; // 重置倒计时
            }
            
            // 2. 捕捉松开 (上升沿) -> 此时发串口最稳
            if (Key1 == 1 && last_key == 0) { 
                if(System_State == 1) Uart_SendString(">> Mode: ALARM ON\r\n");
                else Uart_SendString(">> Mode: SECURITY\r\n");
            }
            
            last_key = Key1; // 更新状态
        }

        // --- 3. 倒计时秒数处理 (借用定时器0产生的1秒标志) ---
        // 这里可以加上你之前的 T1_Count 逻辑
    }
}
