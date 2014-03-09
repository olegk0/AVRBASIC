
//#include <avr/wdt.h>
#define XTAL F_CPU
#include <stdint.h>
//#include "vars1.h"

/*************************************************************************
 delay loop for small accurate delays: 16-bit counter, 4 cycles/loop
*************************************************************************/
static inline void _delayFourCycles(unsigned int __count)
{
    if ( __count == 0 )    
        __asm__ __volatile__( "rjmp 1f\n 1:" );    // 2 cycles
    else
        __asm__ __volatile__ (
    	    "1: sbiw %0,1" "\n\t"                  
    	    "brne 1b"                              // 4 cycles/loop
    	    : "=w" (__count)
    	    : "0" (__count)
    	   );
   
}


/************************************************************************* 
delay for a minimum of <us> microseconds
the number of loops is calculated at compile-time from MCU clock frequency
*************************************************************************/
#define delay(us)  _delayFourCycles( ( ( 1*(XTAL/4000) )*us)/1000 )

void delay_ms0(unsigned char ms);
void delay_ms(unsigned int ms);
void delay_s(unsigned char s);

struct m2bytes{
uint8_t one;
uint8_t two;
} ;

void num_to_str(struct m2bytes *nbuf);

void num_to_lcd(uint8_t fs,uint8_t num);