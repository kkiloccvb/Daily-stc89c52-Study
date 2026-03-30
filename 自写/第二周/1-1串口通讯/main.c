#include <REGX52.H>

// 串口初始化函数：4800bps @ 11.0592MHz (51最常用的频率)
void Uart_Init() {
    SCON = 0x50;    // 8位数据, 可接收
    PCON |= 0x80;   // 波特率倍增
    
    // 配置定时器1用于产生波特率
    TMOD &= 0x0F;   // 清除T1控制位
    TMOD |= 0x20;   // 设定T1为8位自动重装方式
    TL1 = 0xF4;     // 设定定时初值 (4800波特率)
    TH1 = 0xF4;     // 设定定时器重装值
    ET1 = 0;        // 禁止T1中断 (因为它只负责心跳)
    TR1 = 1;        // 启动T1
}

// 发送一个字节
void Uart_SendByte(unsigned char Byte) {
    SBUF = Byte;        // 将数据丢进串口缓冲寄存器
    while(TI == 0);    // 等待发送完成 (TI由硬件置1)
    TI = 0;             // 软件清零TI
}

void main() {
    Uart_Init();
    while(1) {
        Uart_SendByte('H');
        Uart_SendByte('a');
        Uart_SendByte('c');
        Uart_SendByte('h');
        Uart_SendByte('i');
        Uart_SendByte('m');
				Uart_SendByte('i');
				Uart_SendByte(' ');
        
        // 简单延时一下，防止发太快电脑卡死
        // 实际上建议用我们上周学的定时器标志位来控制发送节奏
    }
}