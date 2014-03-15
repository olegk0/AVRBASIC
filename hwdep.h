/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

#define KBD_TAB		0x9
#define KBD_ENTER	0x0d
#define KBD_BS		0x7f
#define KBD_ESC		0x1b
#define KBD_UP		0xb
#define KBD_LEFT	0x8
#define KBD_DOWN	0xa
#define KBD_RIGHT	0x15


#ifdef AVR
//*****************SW SPI**************************
#define SPI_DDR DDRB
#define SPI_PIN PINB
#define SPI_PORT PORTB
#define SMOSI 3		// MOSI *
#define SCLK 5		// CLK *
#define SMISO 4		// MISO *
//*****************LCD**************************
#define A0_DDR DDRC
//#define A0_PIN PINC
#define A0_PORT PORTC
#define A0 2
//***********************BEEP******************
#define BP_DDR DDRB
#define BP_PORT PORTB
#define BPO 0
//*********************************************
#define CS_DDR DDRC
#define CS_PORT PORTC

enum{
	CS_NONE=0,
	CS_LCD=1,
	CS_SD=2,
	CS_ALL=3,
};
//*************************************************
#define DISPTH LCDTHEIGHT //display height for text mode 

#define BREAK_KEY KBD_ESC
#define EDIT_KEY 5

void spiwrite(uint8_t dt);
uint8_t spiread(void);
void spiswitch(uint8_t sel);


//on 16MHz
#define t2_on_16us() { TCNT2 = 0; TCCR2B=0x01; }
#define t2_on_128us() { TCNT2 = 0; TCCR2B=0x02; }
#define t2_on_512us() { TCNT2 = 0; TCCR2B=0x03; }
#define t2_on_1ms() { TCNT2 = 0; TCCR2B=0x04; }
#define t2_on_2ms() { TCNT2 = 0; TCCR2B=0x05; }
#define t2_on_4ms() { TCNT2 = 0; TCCR2B=0x06; }
#define t2_on_16ms() { TCNT2 = 0; TCCR2B=0x07; }

#define t2_off() { TCCR2B = 0; }
#define t2_overflow() ( TCCR2B==0 )

#else

#define DISPTH 20 //display height for text mode 

#define KBD_ENTER 0x0a

#define BREAK_KEY 2 //Ctrl+B

#define EDIT_KEY 5 //Ctrl+E

#endif

#define ALT_FLG		1
#define CTRL_FLG	2
#define SHIFT_FLG	4
#define CL_FLG		8 //Caps lock as language switch

void lputchar(char ch);
void lputint(int num);
void Beep(unsigned int len,signed char tone);//ms, tone (-48(do sub octave);71(si 6 oktave))
