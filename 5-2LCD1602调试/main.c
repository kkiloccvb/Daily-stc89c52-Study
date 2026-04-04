#include <REGX52.H>
#include "LCD1602.h"

void main()
{
	LCD_Init();
	LCD_ShowChar(1,1,'c');
	LCD_ShowChar(1,2,'c');
	LCD_ShowChar(1,3,'b');
	LCD_ShowString(1,5,"Hello");
	LCD_ShowNum(2,1,123,3);
	while(1)
	{
		 
	}
}