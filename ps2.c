/**
 * AVRBASIC (c) 2014 olegvedi@gmail.com 

 * Driver for a PS/2 mouse
 
 * this ps/2 interface is based on avr appnote 313
 * and http://www.computer-engineering.org/index.php?title=Main_Page
 *
 * very helpful was: http://www.mikrocontroller.net/topic/112361
 * and the asociated source code.
 *
 * data and clk state is high
 * 11 bit frame: 1 start bit, 8 bit data lsb first, 1 bit parity (odd), stop bit
 * 
 * start		lsb	data	msb		parity	  stop
 * 0			x x x x x x x x		x		  1
 * 
 * (c) Jens Carroll, Inventronik GmbH
 */

#define SET_KEYBOARD_INDICATORS 0xED

#define PS2_BUFF_SIZE 5
#define FALSE 0
#define TRUE (!FALSE)
#define SET_BIT(data, port, pin) ((data & 0x01) ? (port | _BV(pin)) : (port & ~_BV(pin)))
#define RESET_BIT(data, port, pin) ((data & 0x01) ? (port & ~_BV(pin)) : (port | _BV(pin)))

#define PS2_CLOCK		PD2
#define PS2_PORT		PORTD
#define PS2_PIN			PIND
#define PS2_DDR			DDRD
#define PS2_DATA		PD3

static volatile uint8_t ps2_data,skip_next;						/* holds the received data */
static volatile uint8_t edge, bitcount;			 /* edge: 0 = neg.	1 = pos. */
static volatile uint8_t *in_ptr, *out_ptr;
static volatile uint8_t ps2_buffcnt;
static uint8_t ps2_buffer[PS2_BUFF_SIZE];

const uint8_t Keymap0[] PROGMEM = {//Latin
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, '`', 0,
	0, 0 , 0 , 0, 0 , 'q', '1', 0,
	0, 0, 'z', 's', 'a', 'w', '2', 0,
	0, 'c', 'x', 'd', 'e', '4', '3', 0,
	0, ' ', 'v', 'f', 't', 'r', '5', 0,
	0, 'n', 'b', 'h', 'g', 'y', '6', 0,
	0, 0, 'm', 'j', 'u', '7', '8', 0,
	0, ',', 'k', 'i', 'o', '0', '9', 0,
	0, '.', '/', 'l', ';', 'p', '-', 0,
	0, 0, '\'', 0, '[', '=', 0, 0,
	0 , 0 , KBD_ENTER , ']', 0, '\\', 0, 0,
	0, 0, 0, 0, 0, 0, KBD_BS, 0,
	0, '1', 0, '4', '7', 0, 0, 0,
	'0', '.', '2', '5', '6', '8', KBD_ESC, 0 ,
	0, '+', '3', '-', '*', '9', 0, 0};

const uint8_t Keymap1[] PROGMEM = {//Latin with shift
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, '~', 0,
	0, 0 , 0 , 0, 0 , 'Q', '!', 0,
	0, 0, 'Z', 'S', 'A', 'W', '@', 0,
	0, 'C', 'X', 'D', 'E', '$', '#', 0,
	0, ' ', 'V', 'F', 'T', 'R', '%', 0,
	0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
	0, 0, 'M', 'J', 'U', '&', '*', 0,
	0, '<', 'K', 'I', 'O', ')', '(', 0,
	0, '>', '?', 'L', ':', 'P', '_', 0,
	0, 0, '"', 0, '{', '+', 0, 0,
	0 , 0 , KBD_ENTER , '}', 0, '|', 0, 0,
	0, 0, 0, 0, 0, 0, KBD_BS, 0,
	0, '1', 0, '4', '7', 0, 0, 0,
	'0', '.', '2', '5', '6', '8', KBD_ESC, 0 ,
	0, '+', '3', '-', '*', '9', 0, 0};

const uint8_t Keymapr0[] PROGMEM = {//Cyrillic 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, '¸', 0,
	0, 0 , 0 , 0, 0 , 'é', '1', 0,
	0, 0, 'ÿ', 'û', 'ô', 'ö', '2', 0,
	0, 'ñ', '÷', 'â', 'ó', '4', '3', 0,
	0, ' ', 'ì', 'à', 'å', 'ê', '5', 0,
	0, 'ò', 'è', 'ð', 'ï', 'í', '6', 0,
	0, 0, 'ü', 'î', 'ã', '7', '8', 0,
	0, 'á', 'ë', 'ø', 'ù', '0', '9', 0,
	0, 'þ', '.', 'ä', 'æ', 'ç', '-', 0,
	0, 0, 'ý', 0, 'õ', '=', 0, 0,
	0 , 0 , KBD_ENTER , 'ú', 0, '\\', 0, 0,
	0, 0, 0, 0, 0, 0, KBD_BS, 0,
	0, '1', 0, '4', '7', 0, 0, 0,
	'0', '.', '2', '5', '6', '8', KBD_ESC, 0,
	0, '+', '3', '-', '*', '9', 0, 0};

const uint8_t Keymapr1[] PROGMEM = {//Cyrillic with shift
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, '¨', 0,
	0, 0 , 0 , 0, 0 , 'É', '!', 0,
	0, 0, 'ß', 'Û', 'Ô', 'Ö', '"', 0,
	0, 'Ñ', '×', 'Â', 'Ó', ';', '¹', 0,
	0, ' ', 'Ì', 'À', 'Å', 'Ê', '%', 0,
	0, 'Ò', 'È', 'Ð', 'Ï', 'Í', ':', 0,
	0, 0, 'Ü', 'Î', 'Ã', '?', '*', 0,
	0, 'Á', 'Ë', 'Ø', 'Ù', ')', '(', 0,
	0, 'Þ', ',', 'Ä', 'Æ', 'Ç', '_', 0,
	0, 0, 'Ý', 0, 'Õ', '+', 0, 0,
	0 , 0 , KBD_ENTER , 'Ú', 0, '/', 0, 0,
	0, 0, 0, 0, 0, 0, KBD_BS, 0,
	0, '1', 0, '4', '7', 0, 0, 0,
	'0', '.', '2', '5', '6', '8', KBD_ESC, 0 ,
	0, '+', '3', '-', '*', '9', 0, 0};


uint8_t ps2_send_cmd(uint8_t data);

/*
 * Clear  fifo
 */
void ps2_clear_buffer(void)
{
	in_ptr = out_ptr = ps2_buffer;
	ps2_buffcnt = 0;
}

/*
 * Initialize the PS/2 
 */
void ps2_minit(void)
{
//cli();
//	EIMSK &= ~_BV(INT0);
	EICRA |= _BV(ISC01);	/* INT0 interrupt on falling edge */
	EICRA &= ~_BV(ISC00);
	edge = 0;				/* 0 = falling edge  1 = rising edge */
	bitcount = 11;
	ps2_data = 0;
//	skip_next = FALSE;
//	ps2_clear_buffer();
//	EIFR = _BV(INTF0);
//	EIMSK |= _BV(INT0);
//sei();
}

void ps2_init(void)
{
	EIMSK &= ~_BV(INT0);		
	/* MS clock and data as input */
	PS2_DDR  &= ~_BV(PS2_DATA);
	PS2_DDR &= ~_BV(PS2_CLOCK);
	/* MS clock and data to low */
	PS2_PORT  &= ~_BV(PS2_DATA);
	PS2_PORT &= ~_BV(PS2_CLOCK);
	ps2_clear_buffer();
	ps2_minit();
	skip_next = FALSE;
	TIMSK2 |= _BV(TOIE2); /* allow timer2 overflow */
	EIMSK |= _BV(INT0);		/* enable irpt 0 */

}

static void store_keyb_status(uint8_t prev,uint8_t fl)
{
	if(prev == 0xF0)//release
		keymode &= ~fl;
	else
		keymode |= fl;
}

static void ps2buf_put(uint8_t c)
{
	// put character into buffer and incr ptr 

	if(c >= 0xE0){
		skip_next = c;
		return;
	}
	switch(c){
	case 0x58://Caps Lock 
		if(skip_next != 0xF0){
			ps2_send_cmd(SET_KEYBOARD_INDICATORS);
			if(keymode&CL_FLG){
				keymode &= ~CL_FLG;
				ps2_send_cmd(0);
			}else{
				keymode |= CL_FLG;
				ps2_send_cmd(5);
			}
		}
		skip_next = 0;
		return;
	case 0x14://Ctrl (R+E0) 
		store_keyb_status(skip_next,CTRL_FLG);
		skip_next = 0;
		return;
	case 0x11://Alt (R+E0)
		store_keyb_status(skip_next,ALT_FLG);
		skip_next = 0;
		return;
	case 0x59://RShift
	case 0x12://LShift
		store_keyb_status(skip_next,SHIFT_FLG);
		skip_next = 0;
		return;
	}

	if(skip_next){
		skip_next = 0;
		return;
	}
	if(c > 0x7F){
		return;
	}

	*in_ptr++ = c;
	// pointer wrapping
	if (in_ptr >= ps2_buffer + PS2_BUFF_SIZE)
		in_ptr = ps2_buffer;
	ps2_buffcnt++;
}

uint8_t ps2buf_get(void)
{
	uint8_t byte = 0, *lp;

	while (ps2_buffcnt == 0); // wait for data
	byte = *out_ptr++;	  // get byte

	if (out_ptr >= ps2_buffer + PS2_BUFF_SIZE) // pointer wrapping
		out_ptr = ps2_buffer;
	ps2_buffcnt--;	// decrement buffer count 

	if(keymode&CL_FLG){
		if(keymode&SHIFT_FLG)
			lp = (uint8_t *)&(Keymapr1[byte]);
		else
			lp = (uint8_t *)&(Keymapr0[byte]);
	}else{
		if(keymode&SHIFT_FLG)
			lp = (uint8_t *)&(Keymap1[byte]);
		else
			lp = (uint8_t *)&(Keymap0[byte]);
	}

	byte = pgm_read_byte(lp);
	return byte;
}


ISR(INT0_vect)
{
	t2_on_16ms()
//		rstfl = 1;
//cli();

		if (!edge) {								/* routine entered at falling edge */
			if (bitcount < 11 && bitcount > 2) {	/* bit 3 to 10 is data. Parity bit, */
												/* start and stop bits are ignored. */
				ps2_data >>= 1;
				if ((PS2_PIN & _BV(PS2_DATA)))
					ps2_data |=  0x80;					/* store a '1' */
			}
			EICRA |= _BV(ISC00);					/* set interrupt on rising edge */
			edge = 1;
		} else	{									/* routine entered at rising edge */
			EICRA &= ~_BV(ISC00);					/* set interrupt on falling edge */
			edge = 0;
			bitcount--;
			if (bitcount == 0) {					/* all bits received */
				ps2buf_put(ps2_data);					/* Add data to buffer */
				ps2_data = 0;
				bitcount = 11;
			}
		}
//sei();
	t2_on_16ms()
}

ISR(TIMER2_OVF_vect)
{
cli();
    TCCR2B = 0; /* stop timer2 and reset TCCR2 to indicate the overflow */
    TCNT2 = 0;

	ps2_minit();
sei();
}


/*
 * Send one byte to the ps2 device (here mouse)
 *
 * returns TRUE if no timeout occurred and the device responds with ACK
 * otherwise FALSE
 */
static uint8_t ps2_send_byte(uint8_t data)
{
	uint8_t j, result = FALSE, parity = 0;

//	if (!t2_overflow())
//		return FALSE;		/* send in progress */

	EIMSK &= ~_BV(INT0);	/* disable INT0 */

	/* MS clock and data to high */
	PS2_DDR &= ~_BV(PS2_CLOCK);
	PS2_DDR &= ~_BV(PS2_DATA);

	/* MS clock now to low */
	PS2_DDR |= _BV(PS2_CLOCK);

	/* minimum delay between clock low and data low */
	delay(120);

	/* next MS data to low */
	PS2_DDR  |= _BV(PS2_DATA);

	/* send start bit (just with this delay) */
	delay(20);

	/* release MS clock as input - hi*/
	PS2_DDR &= ~_BV(PS2_CLOCK);
	delay(50);

	j = 0;
	t2_on_16ms();

	do {
		/* wait until data gets low (ack from device) */
		while (PS2_PIN & _BV(PS2_CLOCK))
			if (t2_overflow()) break;

		if (j<8) {
			PS2_DDR = RESET_BIT(data, PS2_DDR, PS2_DATA);
			if (data & 0x01) {
				parity ^= 0x01;
			}

			data >>= 1;
		} else if (j==8) {
			/* insert parity */
			PS2_DDR = RESET_BIT(~parity, PS2_DDR, PS2_DATA);
		} else if (j>8) {
			/* MS clock and data as inputs again */
			PS2_DDR &= ~_BV(PS2_DATA);
			PS2_DDR &= ~_BV(PS2_CLOCK);

			if (j==10) {	
				/* receive ACK eventually
				   wait until data gets low (ack from device) */
				while ((PS2_PIN & _BV(PS2_DATA)) && !t2_overflow());
				if (!t2_overflow())
					result = TRUE;

				while ((PS2_PIN & _BV(PS2_DATA)) && (PS2_PIN & _BV(PS2_CLOCK)) && !t2_overflow());
				if (t2_overflow())
					result = FALSE;
				break;
			}
		}
		
		/* wait until clock gets high or timeout */
		while ((!(PS2_PIN & _BV(PS2_CLOCK))) && !t2_overflow());
		if (t2_overflow())
			break;
		j++;
	} while (j<11);

	/* MS clock and data as input */
	PS2_DDR &= ~_BV(PS2_DATA);
	PS2_DDR &= ~_BV(PS2_CLOCK);

	/* clear interrupt flag bit (write a 1) to prevent ISR entry upon irpt enable */

	ps2_minit();
	ps2_clear_buffer();
	EIFR = _BV(INTF0);
	/* enable ps2 irpt */
	EIMSK |= _BV(INT0);

	/* stop timer */
	t2_off();

	return result;
}

/*
 * Send one byte to the ps2 device (here mouse) and wait for
 * an ACK (0xFA)
 */

uint8_t ps2_send_cmd(uint8_t data)
{
	uint8_t result = FALSE;

	if (ps2_send_byte(data)) {
		delay(50);

		// did we receive ACK before timeout?
		if (ps2_buffcnt!=0 && ps2buf_get()==0xFA)
			result = TRUE;
	}
	return result;
}

