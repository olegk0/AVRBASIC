/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/
#ifdef AVR
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "vars.h"
#include "stlcd.h"
#include "hwdep.h"

#ifdef AVR
#include "subs.h"
//#include "stlcd.h"
#include "util.h"
#include "mmc.c"
#include "ps2.c"
#endif
#include "hwdep.c"
#include "funcs.c"
#include "exppars.c"
#include "run.c"

uint8_t CheckOvrPrint(void)
{
	uint8_t cf=0;

	if(gy >= (DISPTH-1)){
		gy=0;
		serror(EMORE,0);
		if(lgetchar()==BREAK_KEY){
			cf = EINTERUPT;
			gotocmdline();
			return cf;
		}
		else gotocmdline();
	}
	return cf;
}

int main(void)
{
	uint8_t cf,cp;
#ifdef AVR
	wdt_reset();
	wdt_disable();
	TCCR1B = 1; //start timer1
#endif
	init();

	lFputs(mtext);
	lgetchar();
#ifdef AVR
	TCCR1B = 0; //stop timer1
	srand(TCNT1);
#endif
	ClearScreen();
	lputchar('\n');
    while(1){

#include "input.c"

	gotocmdline();
	cf=0;
	switch((uint8_t)CmdInp[0]){
	case RUN:
	    ResetEnv();
//programm loop
	    PrgLineP = FirstPrgLine;
	    cf = 1;
		pkey = 0;
	    while(PrgLineP){
			cp = lgetc();
			if(cp)
				pkey = cp;
			if(cf<2){
			    strcpy((char *)CmdInp,(const char *)(PrgLineP->line));
			    CLine = PrgLineP->lnum;
			}

//print f("Line:%d CMD:%s - %s\n",line,look_cname(CmdInp[0]),CmdInp + 1);
			cf=GoLine();
			if(!cf)
			    PrgLineP = PrgLineP->next;
			if(pkey==BREAK_KEY){
				cf= EINTERUPT;
				break;
			}
		} /* end while line */
		lputint(CLine);
	    break;
	case LIST:
		ClearScreen();
//		lputchar('\n');
	    gp=CmdInp+1;
		gy=0;
		PrgLineP = GetPrgLine(ExpPars1(),1);// line with num >= 
		while(PrgLineP){
		    print_code(NULL,0);
			lputchar('\n');
			cf=CheckOvrPrint();
			if(cf== EINTERUPT) break;
		    PrgLineP = PrgLineP->next;
		}
	    break;
	case HELP:
		ClearScreen();
//		lputchar('\n');
		if(CmdInp[1]=='f'){//c-commands, f-func
			pmode=FNMODE;
			serror(EFUNCS,1);
		}
		else{
			pmode=CONMODE | PRGMODE;
			serror(ECMDS,1);
		}
		lputchar('\n');
		gy=0;
		CLine=0;
#ifdef AVR
		while(cp=pgm_read_byte(&(table_key[CLine].tok))){
			if(pgm_read_byte(&(table_key[CLine].mode))&pmode){
				lputchar('<');
				lputchar(pgm_read_byte(&(table_key[CLine].key_code)));
				lputchar('>');
				lputchar(' ');
				if(pgm_read_byte(&(table_key[CLine].mode))&ALTKEY)
#else
		while(cp=table_key[CLine].tok){
			if(table_key[CLine].mode&pmode){
				lputchar('<');
				lputchar(table_key[CLine].key_code);
				lputchar('>');
				lputchar(' ');
				if(table_key[CLine].mode&ALTKEY)
#endif
					serror(EALT,1);
				CmdInp[0] = cp;
				CmdInp[1] = 0;
				cp = print_code(CmdInp, 255);
				if(pmode==FNMODE){
					gp = (uint8_t *)(table_fn[cp].templ);
					lputchar(' ');
				}
				else
					gp = (uint8_t *)(table_cmd[cp].templ);
#ifdef AVR
				while((cp=pgm_read_byte(gp)) != 0){
#else
				while((cp=*gp) != 0){
#endif
					if(SYMISEXTSYM(cp))
					    print_ssym(cp);
					else
						lputchar(cp);
					gp++;
					lputchar(' ');
				}
				lputchar('\n');
				cf=CheckOvrPrint();
				if(cf== EINTERUPT) break;
			}
			CLine++;
		}
	    break;
	case NEW:
	    FreePrg();
	    break ;
	case CLS:
	    ClearScreen();
	    break ;
	case CLEAR:
	    ResetEnv();
	    break ;
/*	case BYE:
	    FreePrg();
	    return 0 ;
	    break ;*/
	case MEM:
		lputint(GetUsedMem());
		lputchar(':');
		lputint(MAXPRGSZ);
	    break ;
	case SAVE:
		ReplaceChar(CmdInp+2, '"', 0);
		cf=saveprg((const char *)(CmdInp+2));
	    break;
	case LOAD:
		FreePrg();
		ReplaceChar(CmdInp+2, '"', 0);
		cf=loadprg((const char *)(CmdInp+2));
//		cf=loadprg("lander.pbs");
	    break ;
	case 0://cmd line empty
		DelPrgLine(CLine);
		break;
	default:
		LoadPrgLine(CLine);
	    print_code(NULL,0);
		lputchar('\n');
	}
//	if(!cf)
//		serror(EOK,1);
	serror(cf,1);
//	lputline("");
	lgetchar();

    }

    return 0;
}

