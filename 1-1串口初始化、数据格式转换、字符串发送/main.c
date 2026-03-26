#include "REG52.H"  // 51单片机寄存器定义头文件

// 函数声明（必须放在最前面，避免重定义/未声明）
void View(unsigned long u32ViewData);          
void to_BufferData(unsigned long u32Data,unsigned char *pu8Buffer,unsigned char u8Type); 
void SendString(unsigned char *pu8String);     
void UART_Init(); // 新增：串口初始化函数（只执行一次）

// 全局变量：记录发送的变量序号（替代原静态变量，避免逻辑混乱）
unsigned char g_u8SerialNumber = 1;

// 串口初始化（仅执行一次，放在main开头）
void UART_Init()
{
    SCON   = 0x50;    // 串口模式1（8位UART，可变波特率），允许接收
    TMOD  &= 0x0F;    // 清空定时器1的模式位
    TMOD  |= 0x20;    // 定时器1模式2（8位自动重装），定时器0保持默认（不用）
    TH1    = 0xFD;    // 11.0592MHz晶振，9600波特率（精准值）
    TL1    = 0xFD;    
    TR1    = 1;       // 启动定时器1
    ES     = 0;       // 关闭串口中断（查询模式）
    TI     = 0;       // 清除发送标志
    RI     = 0;       // 清除接收标志
}

// 串口发送字符串（核心修复：用TI标志位，而非固定延时）
void SendString(unsigned char *pu8String)
{
    unsigned int Su16SendCnt = 0; // 取消static，避免累计错误
    
    while(pu8String[Su16SendCnt] != 0) // 遍历到字符串结束符
    {
        SBUF = pu8String[Su16SendCnt]; // 写入要发送的字符
        while(!TI);                    // 等待发送完成（核心！替代固定延时）
        TI = 0;                        // 清除发送标志
        Su16SendCnt++;
    }
}

// 数据转字符串缓冲区（原逻辑保留，修复序号赋值）
void to_BufferData(unsigned long u32Data,unsigned char *pu8Buffer,unsigned char u8Type)
{
    // UTF-8中文常量（"第X个数"、"十进制:"等）
    code unsigned char Cu8Array1[]={0xE7,0xAC,0xAC,0x30,0xE4,0xB8,0xAA,0xE6,0x95,0xB0,0x00}; // 第0个数
    code unsigned char Cu8Array2[]={0xE5,0x8D,0x81,0xE8,0xBF,0x9B,0xE5,0x88,0xB6,0x3A,0x00}; // 十进制:
    code unsigned char Cu8Array3[]={0xE5,0x8D,0x81,0xE5,0x85,0xAD,0xE8,0xBF,0x9B,0xE5,0x88,0xB6,0x3A,0x00}; // 十六进制:
    code unsigned char Cu8Array4[]={0xE4,0xBA,0x8C,0xE8,0xBF,0x9B,0xE5,0x88,0xB6,0x3A,0x00}; // 二进制:
    
    unsigned int Su16BufferCnt, Su16TempCnt;
    unsigned long Su32Temp1, Su32Temp2, Su32Temp3;
    unsigned char Su8ViewFlag=0;

    if(1==u8Type) // 类型1：中文序号
    {
        // 复制"第X个数"到缓冲区（替换X为当前序号）
        for(Su16BufferCnt=0; Su16BufferCnt<9; Su16BufferCnt++)
        {
            pu8Buffer[Su16BufferCnt] = Cu8Array1[Su16BufferCnt];
        }
        pu8Buffer[3] = g_u8SerialNumber + '0'; // 替换"第0个数"的0为当前序号
        pu8Buffer[9]  = 0x0d; // 回车
        pu8Buffer[10] = 0x0a; // 换行
        pu8Buffer[11] = 0;    // 结束符
        g_u8SerialNumber++;   // 序号自增
        return;
    }
    else if(2==u8Type) // 十进制
    {
        for(Su16BufferCnt=0; Su16BufferCnt<10; Su16BufferCnt++)
        {
            pu8Buffer[Su16BufferCnt] = Cu8Array2[Su16BufferCnt];
        }
        Su32Temp1=1000000000;
        Su32Temp2=10;
        Su16TempSet=10;
    }
    else if(3==u8Type) // 十六进制
    {
        for(Su16BufferCnt=0; Su16BufferCnt<13; Su16BufferCnt++)
        {
            pu8Buffer[Su16BufferCnt] = Cu8Array3[Su16BufferCnt];
        }
        Su32Temp1=0x10000000;
        Su32Temp2=0x10;
        Su16TempSet=8;
    }
    else // 二进制
    {
        for(Su16BufferCnt=0; Su16BufferCnt<10; Su16BufferCnt++)
        {
            pu8Buffer[Su16BufferCnt] = Cu8Array4[Su16BufferCnt];
        }
        Su32Temp1=0x80000000;
        Su32Temp2=2;
        Su16TempSet=32;
    }

    // 数值转换逻辑（原逻辑保留）
    Su8ViewFlag=0;
    for(Su16TempCnt=0;Su16TempCnt<Su16TempSet;Su16TempCnt++)
    {
        Su32Temp3=u32Data/Su32Temp1%Su32Temp2;
        if(Su32Temp3<10)
        {
            pu8Buffer[Su16BufferCnt]=Su32Temp3+'0';
        }
        else
        {
            pu8Buffer[Su16BufferCnt]=Su32Temp3-10+'A';
        }
        if(0==u32Data)
        {
            Su16BufferCnt++;
            break;
        }
        else if(0==Su8ViewFlag)
        {
            if('0'!=pu8Buffer[Su16BufferCnt])
            {
                Su8ViewFlag=1;
                Su16BufferCnt++;
            }
        }
        else
        {
            Su16BufferCnt++;
        }
        Su32Temp1=Su32Temp1/Su32Temp2;
    }
    pu8Buffer[Su16BufferCnt] = 0x0d;
    pu8Buffer[Su16BufferCnt+1] = 0x0a;
    pu8Buffer[Su16BufferCnt+2] = 0;
}

// View函数（修复重复定义，简化逻辑）
void View(unsigned long u32ViewData)
{
    static unsigned char Su8ViewBuffer[43];
    code unsigned char Cu8_0D_0A[]={0x0d,0x0a,0x00};
    // UTF-8"开始..."
    code unsigned char Cu8Start[]={0xE5,0xBC,0x80,0xE5,0xA7,0x8B,0xE2,0x80,0xA6,0x00}; 
    static unsigned char Su8FirstFlag=0;

    // 首次执行：发送初始化提示
    if(0==Su8FirstFlag)
    {
        Su8FirstFlag=1;
        // 短延时（等串口稳定）
        for(unsigned int i=0; i<10000; i++);
        SendString(Cu8Start);    
        SendString(Cu8_0D_0A);   
        SendString(Cu8_0D_0A);   
    }

    // 发送序号、十进制、十六进制、二进制
    to_BufferData(u32ViewData, Su8ViewBuffer, 1);
    SendString(Su8ViewBuffer);
    to_BufferData(u32ViewData, Su8ViewBuffer, 2);
    SendString(Su8ViewBuffer);
    to_BufferData(u32ViewData, Su8ViewBuffer, 3);
    SendString(Su8ViewBuffer);
    to_BufferData(u32ViewData, Su8ViewBuffer, 4);
    SendString(Su8ViewBuffer);
    SendString(Cu8_0D_0A);
}

// 主函数（核心：先初始化串口）
void main() 
{
    unsigned char a; // RAM随机值
    unsigned char b; // RAM随机值
    unsigned char c; // RAM随机值
    unsigned char d=9; // 初始化为9

    UART_Init(); // 第一步：初始化串口（必须最先执行）
    
    b=3; 
    c=b; 

    // 发送4个变量
    View(a); 
    View(b); 
    View(c); 
    View(d); 

    while(1) // 死循环
    {
    }
}