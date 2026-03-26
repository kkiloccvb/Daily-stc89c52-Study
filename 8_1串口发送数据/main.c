#include <REGX52.H>
#include <INTRINS.H>  
void Uart1_Init(void)	//4800bps@11.0592MHz
{
	PCON |= 0x80;		//使能波特率倍速位SMOD
	SCON = 0x50;		//8位数据,可变波特率
	TMOD &= 0x0F;		//设置定时器模式
	TMOD |= 0x20;		//设置定时器模式
	TL1 = 0xF4;			//设置定时初始值
	TH1 = 0xF4;			//设置定时重载值
	ET1 = 0;			//禁止定时器中断
	TR1 = 1;			//定时器1开始计时
}
void Delay1000ms(void)	//@11.0592MHz
{
	unsigned char data i, j, k;

	_nop_();
	i = 8;
	j = 1;
	k = 243;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
/**
*@brief串口发送一个字节数据
*@param无
*@retval无
*/
void Uart1_SendByte (unsigned char Byte)
{
	SBUF=Byte;
	while(TI==0);
	TI=0;
}

void main()
{
	Uart1_Init();
	while(1)
	{
		Uart1_SendByte(0x66);
		Delay1000ms();
	}
}