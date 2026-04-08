#include <REGX52.H>

typedef unsigned char u8;
typedef unsigned int u16;

// --- 硬件定义 ---
sbit IRIN = P3^2;    // 红外接收头接在 P3.2 (外部中断0)
sbit LED1 = P2^0;    // 假设我们要控制第一颗LED
sbit LED2 = P2^1;    // 假设我们要控制第二颗LED

u8 IrValue[4];       // 存储红外 4 字节数据
u8 IrOK = 0;         // 接收成功标志位

// --- 延时函数 (针对 11.0592MHz) ---
void delay(u16 i) {
    while(i--);
}

// --- 红外初始化 ---
void IrInit() {
    IT0 = 1;    // 下降沿触发
    EX0 = 1;    // 开启外部中断0
    EA  = 1;    // 开启总中断
    IRIN = 1;
}

void main() {
    IrInit();
    LED1 = 1;   // 默认关灯 (51单片机高电平关)
    LED2 = 1;

    while(1) {
        if(IrOK) {      // 如果收到了一帧完整的红外数据
            IrOK = 0;   // 清除标志位

            /* 根据 NEC 协议，IrValue[2] 是数据码。
               你需要根据你的遥控器实际键值来修改下面的十六进制数。
               通常“1”键是 0x0C，“2”键是 0x18 (不同遥控器不同)。
            */
            switch(IrValue[2]) {
                case 0x45: LED1 = ~LED1; break; // 比如按“CH-”键翻转LED1
                case 0x46: LED2 = ~LED2; break; // 比如按“CH”键翻转LED2
                case 0x47: P2 = 0xFF;    break; // 比如按“CH+”键全关
            }
        }
    }
}

// --- 红外中断解码服务函数 ---
void ReadIr() interrupt 0 {
    u8 j, k;
    u16 err;
    u8 time;

    delay(700); // 避开起始码
    if(IRIN == 0) {
        err = 1000;
        while((IRIN == 0) && (err > 0)) { delay(1); err--; }
        if(IRIN == 1) {
            err = 500;
            while((IRIN == 1) && (err > 0)) { delay(1); err--; }
            for(k = 0; k < 4; k++) {
                for(j = 0; j < 8; j++) {
                    err = 60;
                    while((IRIN == 0) && (err > 0)) { delay(1); err--; }
                    err = 500;
                    time = 0;
                    while((IRIN == 1) && (err > 0)) {
                        delay(10); // 约100us
                        time++; err--;
                        if(time > 30) return;
                    }
                    IrValue[k] >>= 1;
                    if(time >= 8) IrValue[k] |= 0x80;
                    time = 0;
                }
            }
        }
        // 校验数据
        if(IrValue[2] != ~IrValue[3]) return;
        IrOK = 1; // 标记解码成功
    }
}