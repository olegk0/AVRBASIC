/*
 AVRBASIC (c) 2014 olegvedi@gmail.com 

ST7565 LCD library!

Copyright (C) 2010 Limor Fried, Adafruit Industries

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 // some of this code was written by <cstone@pobox.com> originally; it is in the public domain.
*/

#include <avr/io.h>
#include "subs.h"
#include <stdlib.h>
#include <string.h>
#include "stlcd.h"
#include <avr/pgmspace.h>
#include "hwdep.h"

#include "glcdfont.c"

//uint8_t scrn_buf[128][64/8]; 

extern uint8_t xt,yt,gy; 

void clear_line(uint8_t pg)
{
	uint8_t c;

    st7565_command(CMD_SET_PAGE | pg);
    st7565_command(CMD_SET_COLUMN_UPPER);
    st7565_command(CMD_SET_COLUMN_LOWER);
    st7565_command(CMD_RMW);
   
	for(c = 0; c < 128; c++) {
		st7565_data(0);
	}
	xt=0;
}

void clear_screen(void)
{
	uint8_t p;
  
	for(p = 0; p < 8; p++) {
		clear_line(p);
	}
	xt=0;
	yt=LCDTHEIGHT-1;
	st7565_command(CMD_SET_DISP_START_LINE | ((LCDTHEIGHT-1)*8)+8);
}


void st7565_init(void) {

	A0_DDR |= _BV(A0);

  delay_ms0(200);

  st7565_command(CMD_NOP);
  // LCD bias select
  st7565_command(CMD_SET_BIAS_7);
  delay_ms0(10);
  // ADC select
  st7565_command(CMD_SET_ADC_NORMAL);
//  st7565_command(CMD_SET_ADC_REVERSE);
  delay_ms0(10);
  // SHL select
//  st7565_command(CMD_SET_COM_NORMAL);
	st7565_command(CMD_SET_COM_REVERSE);
  delay_ms0(10); 
 // Initial display line
  st7565_command(CMD_SET_DISP_START_LINE);

  // turn on voltage converter (VC=1, VR=0, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x4);
  // wait for 50% rising
  delay_ms0(50);

  // turn on voltage regulator (VC=1, VR=1, VF=0)
  st7565_command(CMD_SET_POWER_CONTROL | 0x6);
  // wait >=50ms
  delay_ms0(60);

  // turn on voltage follower (VC=1, VR=1, VF=1)
  st7565_command(CMD_SET_POWER_CONTROL | 0x7);
  // wait
  delay_ms0(10);

  // set lcd operating voltage (regulator resistor, ref voltage resistor)
  st7565_command(CMD_SET_RESISTOR_RATIO | 0x6);

  // initial display line
  // set page address
  // set column address
  // write display data
  delay_ms0(50);
}

void st7565_command(uint8_t c) {
	spiswitch(CS_LCD);
	A0_PORT &= ~_BV(A0);
	spiwrite(c);
	spiswitch(CS_NONE);
}

void st7565_data(uint8_t c) {
	spiswitch(CS_LCD);
	A0_PORT |= _BV(A0);
	spiwrite(c);
	spiswitch(CS_NONE);
}

void st7565_set_brightness(uint8_t val) {
    st7565_command(CMD_SET_VOLUME_FIRST);
    st7565_command(CMD_SET_VOLUME_SECOND | (val & 0x3f));
}


void write_sym(uint8_t x, uint8_t line, uint8_t *buf)
{
  uint8_t c;
	st7565_command(CMD_SET_PAGE | line);
	st7565_command(CMD_SET_COLUMN_UPPER | (x >> 4));
	st7565_command(CMD_SET_COLUMN_LOWER | (x & 0xf));
	st7565_command(CMD_RMW);
    
    for(c = 0; c < LCDSYMWIDTH; c++) {
      st7565_data(buf[c]);
    }
}

void write_scrn(uint8_t x, uint8_t line, uint8_t seg) {

	st7565_command(CMD_SET_PAGE | line);
	st7565_command(CMD_SET_COLUMN_UPPER | (x >> 4));
	st7565_command(CMD_SET_COLUMN_LOWER | (x & 0xf));
	st7565_data(seg);
}
//************************************************************************

void lputc(uint8_t c, uint8_t mode) {
	uint8_t i, sym[LCDSYMWIDTH];

	if(c=='\r') clear_line(yt);//13
	else if((xt+LCDSYMWIDTH) > LCDWIDTH || c=='\n') {//10
		yt++;
		gy++;
		if(yt>=LCDTHEIGHT)
			yt=0;
		st7565_command(CMD_SET_DISP_START_LINE | (yt*8)+8);
		clear_line(yt);
	}
/*uart_putchar('\r');
uart_put_dec(c);
uart_putchar(':');
uart_put_dec(xt);
uart_putchar(':');
uart_put_dec(yt);*/
	if(c<32) return;
	else if(c<130) c -= 32;
	else if(c<192) c = 0;
	else c = c - 94;

	for(i=0;i<5;i++){
		if(mode)//inv
			sym[i] = ~pgm_read_byte(MFont+(c*5)+i);
		else
			sym[i] = pgm_read_byte(MFont+(c*5)+i);
	}
	if(mode)//inv
		sym[i] = 255;
	else
		sym[i] = 0;
	write_sym(xt, yt, sym);
	xt=xt+LCDSYMWIDTH;
}
/*
void set_pos(uint8_t x, uint8_t line)//TODO
{
	xt =x;
	yt=line;
}
*/
//**********************************************************
// the most basic function, set a single pixel
void setpixel(uint8_t x, uint8_t y, uint8_t color) {
  if ((x >= (LCDWIDTH)) || (y >= (LCDHEIGHT)))
    return;
  y = LCDHEIGHT - y;
  // x is which column
  if (color)
	write_scrn( x,y>>3,_BV(7-(y%8)));
//    scrn_buf[x][y>>3] |= _BV(7-(y%8));  
  else
	write_scrn( x,y>>3,0);
//    scrn_buf[x][y>>3] &= ~_BV(7-(y%8)); 
}


// bresenham's algorithm - thx wikpedia
void drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, 
	      uint8_t color) {

  uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  uint8_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int8_t err = dx >> 1;
  int8_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<x1; x0++) {
    if (steep) {
      setpixel(y0, x0, color);
    } else {
      setpixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

// filled rectangle
void fillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
	      uint8_t color) {

  // stupidest version - just pixels - but fast with internal scrn_buf!
  for (uint8_t i=x; i<x+w; i++) {
    for (uint8_t j=y; j<y+h; j++) {
      setpixel( i, j, color);
    }
  }
}


// draw a rectangle
void drawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
	      uint8_t color) {
  // stupidest version - just pixels - but fast with internal scrn_buf!
  for (uint8_t i=x; i<x+w; i++) {
    setpixel( i, y, color);
    setpixel( i, y+h-1, color);
  }
  for (uint8_t i=y; i<y+h; i++) {
    setpixel( x, i, color);
    setpixel( x+w-1, i, color);
  } 
}


// draw a circle
void drawcircle(uint8_t x0, uint8_t y0, uint8_t r, 
	      uint8_t color) {
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -1 * (r<<1);
  int8_t x = 0;
  int8_t y = r;

  setpixel( x0, y0+r, color);
  setpixel( x0, y0-r, color);
  setpixel( x0+r, y0, color);
  setpixel( x0-r, y0, color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    setpixel( x0 + x, y0 + y, color);
    setpixel( x0 - x, y0 + y, color);
    setpixel( x0 + x, y0 - y, color);
    setpixel( x0 - x, y0 - y, color);
    
    setpixel( x0 + y, y0 + x, color);
    setpixel( x0 - y, y0 + x, color);
    setpixel( x0 + y, y0 - x, color);
    setpixel( x0 - y, y0 - x, color);
    
  }
}


// draw a circle
void fillcircle(uint8_t x0, uint8_t y0, uint8_t r, 
	      uint8_t color) {
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -1 * (r<<1);
  int8_t x = 0;
  int8_t y = r;

  for (uint8_t i=y0-r; i<=y0+r; i++) {
    setpixel( x0, i, color);
  }

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    for (uint8_t i=y0-y; i<=y0+y; i++) {
      setpixel( x0+x, i, color);
      setpixel( x0-x, i, color);
    } 
    for (uint8_t i=y0-x; i<=y0+x; i++) {
      setpixel( x0+y, i, color);
      setpixel( x0-y, i, color);
    }    
  }
}

/*
// clear everything
void clear_scrn_buf(void) {
  memset(scrn_buf, 0, 1024);
}
*/
