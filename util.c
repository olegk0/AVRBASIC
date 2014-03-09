// general purpose stuff, use as you'd like!

#include <avr/io.h>
#include <avr/interrupt.h>
#include "subs.h"
#include <avr/pgmspace.h>
#include "util.h"

// Creates a 8N1 UART connect
// remember that the BBR is #defined for each F_CPU in util.h
void uart_init(uint16_t BRR) {
  UBRR0 = BRR;               // set baudrate counter

  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
//  UCSR0C = _BV(USBS0) | (3<<UCSZ00);
  DDRD |= _BV(1);
  DDRD &= ~_BV(0);
}


// Some uart functions for debugging help
int uart_putchar(char c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  return 0;
}

char uart_getchar(void) {
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

char uart_getch(void) {
  return (UCSR0A & _BV(RXC0));
}

void uart_puts(const char* str)
{
  while(*str)
    uart_putc(*str++);
}


void uart_putc_hex(uint8_t b)
{
  /* upper nibble */
  if((b >> 4) < 0x0a)
    uart_putc((b >> 4) + '0');
  else
    uart_putc((b >> 4) - 0x0a + 'a');

  /* lower nibble */
  if((b & 0x0f) < 0x0a)
    uart_putc((b & 0x0f) + '0');
  else
    uart_putc((b & 0x0f) - 0x0a + 'a');
}

void uart_putw_hex(uint16_t w)
{
  uart_putc_hex((uint8_t) (w >> 8));
  uart_putc_hex((uint8_t) (w & 0xff));
}

void uart_putdw_hex(uint32_t dw)
{
  uart_putw_hex((uint16_t) (dw >> 16));
  uart_putw_hex((uint16_t) (dw & 0xffff));
}

void uart_putw_dec(uint16_t w)
{
  uint16_t num = 10000;
  uint8_t started = 0;

  while(num > 0)
    {
      uint8_t b = w / num;
      if(b > 0 || started || num == 1)
	{
	  uart_putc('0' + b);
	  started = 1;
	}
      w -= b * num;

      num /= 10;
    }
}

void uart_put_dec(int8_t w)
{
  uint16_t num = 100;
  uint8_t started = 0;

  if (w <0 ) {
    uart_putc('-');
    w *= -1;
  }
  while(num > 0)
    {
      int8_t b = w / num;
      if(b > 0 || started || num == 1)
	{
	  uart_putc('0' + b);
	  started = 1;
	}
      w -= b * num;

      num /= 10;
    }
}

void uart_putdw_dec(uint32_t dw)
{
  uint32_t num = 1000000000;
  uint8_t started = 0;

  while(num > 0)
    {
      uint8_t b = dw / num;
      if(b > 0 || started || num == 1)
	{
	  uart_putc('0' + b);
	  started = 1;
	}
      dw -= b * num;

      num /= 10;
    }
}
