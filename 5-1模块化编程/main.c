#include <REGX52.H>
#include " Delay.h"


void main ()
{
	while(1)
	{
		P2_0=1;
		Delay(500);
		P2_0=0;
		Delay(500);
	}
}