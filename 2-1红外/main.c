#include <REGX52.H>

typedef unsigned int u16;
typedef unsigned char u8;

// --- 硬件引脚定义 (对照A2原理图) ---
sbit LSA  = P2^2;          // 138译码器输入A
sbit LSB  = P2^3;          // 138译码器输入B
sbit LSC  = P2^4;          // 138译码器输入C
sbit IRIN = P3^2;          // 红外接收头OUT脚 (INT0外部中断)

// --- 全局变量 ---
u8 IrValue[4];             // 存储4字节红外数据：地址、地址反码、命令、命令反码
u8 DisplayData[8];         // 数码管显示缓冲区
u8 code smgduan[17] = {    // 共阴极数码管段码
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x76
};

/**
 * 软件延时函数
 * 注意：此函数在中断中使用时，其时间计算受晶振频率影响极大
 */
void delay(u16 i) {
    while(i--);
}

/**
 * 数码管动态扫描显示
 */
void DigDisplay() {
    u8 i;
    for(i = 0; i < 3; i++) {
        switch(i) {
            case 0: LSA=1; LSB=1; LSC=1; break; // 第0位
            case 1: LSA=0; LSB=1; LSC=1; break; // 第1位
            case 2: LSA=1; LSB=0; LSC=1; break; // 第2位
        }
        P0 = DisplayData[i];  // 传送段码
        delay(100);           // 扫描延时
        P0 = 0x00;            // 消隐，防止重影
    }
}

/**
 * 红外初始化
 */
void IrInit() {
    IT0 = 1;   // 设置外部中断0为下降沿触发
    EX0 = 1;   // 开启外部中断0
    EA  = 1;   // 开启总中断
    IRIN = 1;  // 初始化引脚高电平
}

void main() {
    IrInit();
    while(1) {
        // 将接收到的命令码(IrValue[2])以十六进制显示在数码管上
        DisplayData[0] = smgduan[IrValue[2] / 16]; // 高4位
        DisplayData[1] = smgduan[IrValue[2] % 16]; // 低4位
        DisplayData[2] = smgduan[16];              // 显示"H"或其他标志
        DigDisplay();
    }
}

/**
 * 外部中断0服务函数 - 红外解码核心
 */
void ReadIr() interrupt 0 {
    u8 j, k;
    u16 err;
    u8 time;

    delay(700);        // 延时约7ms，避开9ms起始码的抖动部分
    if (IRIN == 0) {   // 确认起始信号
        
        // 1. 等待9ms低电平结束
        err = 1000;
        while ((IRIN == 0) && (err > 0)) {
            delay(1);
            err--;
        }

        // 2. 等待4.5ms高电平结束
        if (IRIN == 1) {
            err = 500;
            while ((IRIN == 1) && (err > 0)) {
                delay(1);
                err--;
            }

            // 3. 开始接收4个字节数据
            for (k = 0; k < 4; k++) {
                for (j = 0; j < 8; j++) {
                    // 等待560us低电平过去
                    err = 60;
                    while ((IRIN == 0) && (err > 0)) {
                        delay(1);
                        err--;
                    }

                    // 计算高电平长度
                    err = 500;
                    time = 0;
                    while ((IRIN == 1) && (err > 0)) {
                        delay(10); // 约100us单位
                        time++;
                        err--;
                        if (time > 30) return; // 超时退出
                    }

                    // 位移并存入数据
                    IrValue[k] >>= 1;
                    if (time >= 8) {       // 高电平长于0.8ms判断为1
                        IrValue[k] |= 0x80;
                    }
                }
            }
        }
        
        // 4. 校验数据（命令码与命令反码是否匹配）
        if (IrValue[2] != ~IrValue[3]) {
            return;
        }
    }
}