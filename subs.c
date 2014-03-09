#include "subs.h"
#include <avr/pgmspace.h>

void delay_ms0(unsigned char ms)
{
	while(ms>0)
	{
		delay(1000);
		ms--;
	}
//	wdt_reset();
}

void delay_ms(unsigned int ms)
{
	while(ms>0)
	{
		delay(1000);
		ms--;
	}
//	wdt_reset();
}

void delay_s(unsigned char s)
{
	while(s>0)
	{
		delay_ms(1000);
		s--;
	}
//	wdt_reset();
}

