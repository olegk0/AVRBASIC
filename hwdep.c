/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

#ifdef AVR
#include <avr/wdt.h>
#include "integer.h"
#include "pff.h"
#include "diskio.h"

#define	INIT_SPI()	{ SPI_PORT |=_BV(SMOSI)|_BV(SMISO);SPI_PORT &= ~_BV(SCLK); SPI_DDR |=_BV(SMOSI)|_BV(SCLK);SPI_DDR &=  ~_BV(SMISO); }	

#else

#include <termios.h>
#include <signal.h>

struct termios oldsettings, newsettings;

void sig_int(int signo)
{
	puts("\033[?25h");//show cursor
	tcsetattr(fileno(stdin), TCSANOW, &oldsettings);
	exit(0);
}

#endif



void init(void)
{
#ifdef AVR
  uart_init(BRRL_9600);

	INIT_SPI();
	spiswitch(CS_NONE);
	st7565_init();
	st7565_command(CMD_DISPLAY_ON);
	st7565_set_brightness(0x07);
	st7565_command(CMD_SET_ALLPTS_ON);
	clear_screen();
	delay_ms(50);
    st7565_command(CMD_SET_ALLPTS_NORMAL);
	ps2_init();
	sei();
#else
	tcgetattr(fileno(stdin), &oldsettings);
	newsettings = oldsettings;
	newsettings.c_lflag &= ~(ECHO|ICANON);
	newsettings.c_cc[VMIN] = 0;
	newsettings.c_cc[VTIME] = 0;

	signal(SIGINT, sig_int);
	tcsetattr(fileno(stdin), TCSANOW, &newsettings);
	puts("\033[?25l");//hide cursor
	puts("\033[2J");//hide cursor

/*
  printf( "%c[2J", 27 );           // очистили экран 
   fflush( stdout ); 
   printf( "%c[%d;%dH", 27, y, x ); // установили курсор в позицию 
 
      fflush( stdout ); 
*/
#endif
}

#ifdef AVR
uint8_t loadprg(const char *path)
{
FATFS fs;
FRESULT res;
WORD br;

	res = pf_mount(&fs);
	if (res != FR_OK){
		return res;
	}
	res =pf_open(path);
	if (res != FR_OK){
		return res;
	}
	CLine = 0;//counter loaded lines
	res = pf_read(NULL, 512*4, &br);
	lputchar(' ');
	lputint(CLine);
	pf_mount(NULL);
	if(res) res = EERROR;//TODO check
	return res;
}

uint8_t saveprg(const char *path)
{
FATFS fs;
FRESULT res=0;
WORD br;

	PrgLineP = FirstPrgLine;//pointer cur line
	if(PrgLineP){

		res = pf_mount(&fs);
		if (res != FR_OK)
			return res;
		res =pf_open(path);
		if (res != FR_OK)
			return res;

		PrepToSavPrgLine(PrgLineP);//prepare line & copy to CmdInp
		CLine = 0; //as pointer in CmdInp
		res = pf_write(CmdInp, 512*4, &br);

		pf_write(0, 0, &br);//финализируем запись
		pf_mount(NULL);
	}
	return res;
}
#else
uint8_t loadprg(const char *path)
{
FILE *fs;
register uint8_t cp,cf=1;

	fs = fopen(path, "rb");

	if(fs)
	{
		cp=0;
		while(fgets((char *)CmdInp, BMAX, fs)){
			if(CmdInp[0]==0)
				break;
			ReplaceChar(CmdInp, '\n', 0);
		    if(cf=LoadPrgLine(0))
				break;
			cp++;
		}
		fclose(fs);
		lputchar(' ');
		lputint(cp);
	}
	return cf;
}

uint8_t saveprg(const char *path)
{
FILE *fs;
register uint8_t cp,cf=0;

	fs = fopen(path, "wb");
	if(fs)
	{
		cp=0;
		PrgLineP = FirstPrgLine;
		while(PrgLineP){
			PrepToSavPrgLine(PrgLineP);
			fprintf(fs,"%s\n",CmdInp);
		    PrgLineP = PrgLineP->next;
			cp++;
		}
		cf = fprintf(fs,"%c",END);
		fclose(fs);
		lputchar(' ');
		lputint(cp);
	}
	return cf;
}
#endif
void lputchar(char ch)
{
#ifdef AVR
    lputc(ch,0);
#else
	if(ch=='\n') gy++;//for compatibility
    putchar(ch);

#endif
}

void lputs(char *str)
{
	while(str[0] != 0) {
		lputchar(str[0]);
		str++;
	}
}

void lputint(int num)
{
	char TmpS[6];

	sprintf(TmpS,"%d",num);
	lputs(TmpS);
}
//*****************



uint8_t lgetc(void)
{
	register uint8_t ch;
	
#ifdef AVR
//	ch = (uint8_t)uart_getchar();
	if(ps2_buffcnt)
		ch=ps2buf_get();
	else
		ch=0;

	switch(ch){
	case 'c':
		if(keymode&CTRL_FLG){//reset
			wdt_enable(WDTO_120MS);
			while(1);
		}
		break;
	}
	return(ch);
#else
	while(1){
		ch = fgetc(stdin);
		switch(ch){
		case 0xff://none
			ch=0;
			break;
		case 0x1B://Alt key
			if(keymode&ALT_FLG)
				keymode &= ~ALT_FLG;
			else
				keymode |= ALT_FLG;
			ch=0;
			break;
//		default:
//printf("\n%d\n",ch);
		}
		return(ch);
	}
#endif
}

uint8_t lgetchar(void)
{
	register uint8_t ch;
#ifdef AVR
	ps2_clear_buffer();
#else
	keymode = 0;
#endif

	do
		ch = lgetc();
	while(!ch);
#ifdef AVR
/*	switch(ch){
	case 'e':
		if(keymode&CTRL_FLG)
			ch = EDIT_KEY;
		break;
	}*/

//uart_putchar('\r');
//uart_putw_dec(ch);
	Beep(10,30);
#else
//printf("\n%d\n",ch);
#endif
	return ch;
}

void gotocmdline(void)
{
#ifdef AVR
//	clear_lastline();
	lputchar('\r');
#else
	lputchar('\r');
	lputs("                                                            ");//TODO to end of line
	lputchar('\r');
#endif
}

void PutCursor(void)
{

#ifdef AVR
    lputc(cursmode,1);
#else
	lputs("\033[7m");//Bold
	lputchar(cursmode);
	lputs("\033[27m");
#endif
}

void ClearScreen(void)//cls
{
	pmode =0;

#ifdef AVR
	clear_screen();
#else
	register uint8_t li;

	li = 40;//TODO lines
	while(li){
		lputchar('\n');
		li--;
	}
//printf("\033c");
#endif
}

void Beep(unsigned int len,signed char tone)//ms, tone (-48(do sub octave);71(si 6 oktave))
{
	register uint8_t li,lj;
	register unsigned int i;
	register unsigned long l;

	if(tone > 71 || tone < -48)
		return;
	li = tone + 48;
	lj = li/12;//scale
	li = li-(lj*12);//offset
#ifdef AVR
	i = pgm_read_word(&(notes_sub_us[li]));//period us
#else
	i = notes_sub_us[li];//period us
#endif
	if(lj){// 2 ^ scale
		lj--;
		li = 2;
		while(lj){
			li *= 2;
			lj--;
		}
	}
	else li = 1;

	i = i/li; //period us

#ifdef AVR
	l = len;
	l *= 1000;
	l /= i;
	i = i >> 1;
	BP_DDR |= _BV(BPO);
	while(l){
		BP_PORT |= _BV(BPO);
		delay(i);
		BP_PORT &= ~_BV(BPO);
		delay(i);
		l--;
	}
	BP_DDR &= ~_BV(BPO);
#else
	printf("\033[10;%d]",1/i);
	while(len){
		usleep(1000);
		len--;
	}
	printf("\033[10;0]");

#endif
}

void lFputs(const uint8_t *Fstr)//print from flash
{
#ifdef AVR
	register uint8_t c;

	while(c=pgm_read_byte(Fstr)){
		lputchar(c);
		Fstr++;
	}
#else
	lputs((char *)Fstr);
#endif
}
//**************************************
#ifdef AVR
void spiwrite(uint8_t data) {
  int8_t i;

//	SPI_PORT |= _BV(SCLK);
    for ( i = 0; i < 8; i++ ) {

        if ( ( data & 0x80 ) == 0x00 ) SPI_PORT &= ~ _BV(SMOSI);
        else SPI_PORT |= _BV(SMOSI);
            
        data = data << 1; 
        
        SPI_PORT |= _BV(SCLK);
		delay(2);
        SPI_PORT &= ~_BV(SCLK);

    }
//	SPI_PORT &= ~_BV(SCLK);
}

// Send 0xFF and receive a byte
uint8_t spiread(void) {

    uint8_t i, res = 0;

    SPI_PORT |= _BV(SMOSI);
//cli();
    for ( i = 0; i < 8; i++ ) {
        SPI_PORT |= _BV(SCLK);
		delay(1);
        res = res << 1;
        if ( ( SPI_PIN & _BV(SMISO) ) != 0x00 ) res = res | 0x01;

        SPI_PORT &= ~ _BV(SCLK);
		delay(1);

    }
//sei();
//	SPI_PORT |= _BV(SCLK);

    return res;

}	/* Send 0xFF and receive a byte */

void spiswitch(uint8_t sel) {
	CS_DDR |= CS_ALL;
	CS_PORT |= CS_ALL;//all off
	CS_PORT &= ~sel;  
}
#else
void delay_ms(unsigned int dl)
{
	while(dl){
		usleep (1000);
		dl--;
	}
}
#endif

uint8_t in_port(uint8_t pn){
#ifdef AVR
uint8_t ret=0;

	switch(pn){
	case 0:
		ret = (uint8_t)uart_getchar();
		break;
	case 1:
		break;
	}
	return ret;
#else
	return 0;
#endif
}

void out_port(uint8_t port, uint8_t data){
#ifdef AVR

	switch(port){
	case 0:
		uart_putchar(data);
		break;
	case 1:
		break;
	}
	return;
#else
	return;
#endif
}
