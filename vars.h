/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

//***************Version************************
#define MAJOR 0
#define MINOR 1

#define STRING(x) STR_HELPER(x)
#define STR_HELPER(x) #x
#define VERSION STRING(MAJOR) "." STRING(MINOR)
//*****************************************
#ifdef AVR
#define MAXPRGSZ 1200 // max program size
#define BMAX 30 //command buf 
#define volat volatile
#define volat

#else

#define BMAX 30 //command buf 
#include <unistd.h>
#define MAXPRGSZ 2000 // max program size
#define volat

#endif
//******************************************
#define VMAX 'Z'-'A'+1 //variables  +150b
#define SMAX 10 //gosub stack

#define LMAX 10 //max "for" lops
#define LNMAX 255 //max line num

#define TOVAR(x) x-'A'
#define SIZEOFVAR() (((0x1ULL << ((sizeof(int) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0x7ULL << ((sizeof(int) * 8ULL) - 4ULL)))
//*****************************************
const char mtext[]
#ifdef AVR
PROGMEM
#endif 
= "   AVR BASIC v" VERSION "\n(C)olegvedi@gmail.com\nmax program size:\n" STRING(MAXPRGSZ) " b. or " STRING(LNMAX) " ln.\nlist of commands:\nHELP [c|f]";

//****************************************************
#define END  0

//************************************************

enum{
	NONEVAL='?',//?
	CMDVAL='C',//C - command
	EXPVAL='E',//E - expression
	ERRVAL='F',//F - fault
	NUMVAL='N',//N - number
	STRVAL='S',//S - string
	VARVAL='V',//V - variable
	MLTVAL='M',//M - multi val
	OKVAL='O',//O all ok
	okVAL='o',// exp ok
	ALLVAL='A',//any syms
	NOPVAL='n',//num optional
};
//*********************************************

#define TO			0x10
#define THEN		0x11
#define GESIGN 		0x1A
#define LESIGN	  	0x1B
#define NESIGN	  	0x1D

#define EOL			0xA0
#define FINISHED	0xA1


struct symbs {
  uint8_t text[5];
  uint8_t tok;
};
//**************************
const struct symbs table_ssymb[]
#ifdef AVR
PROGMEM
#endif 
= {//"\"", QUOTE,
  ">=", GESIGN,
  "<=", LESIGN,
  "<>", NESIGN,
  "TO", TO,
  "THEN", THEN,
  "", END
};

//**************************
struct commands {
  uint8_t text[8];
  uint8_t tok;
  uint8_t templ[6];
};
//************************
#define SYMISEXTSYM(sym) ((sym)>=0x10 && (sym)<=0x1f)
#define SYMISCHAR(sym) ((sym)>=0x20 && (sym)<=0x7f)
#define SYMISNUM(sym) ((sym)>=0x30 && (sym)<=0x39)
#define SYMISCMD(sym)	((sym)>=0xB0 && (sym)<=0xEF)
#define SYMISFN(sym)	((sym)>=0xF0)
#define SYMISVAR(sym)	((sym)>='A' && (sym)<='Z')

const uint8_t exp_syms[]
#ifdef AVR
PROGMEM
#endif 
 = {LESIGN,GESIGN,NESIGN,'(',')','*','+','-','/','<','=','>',END};

const unsigned int notes_sub_us[]
#ifdef AVR
PROGMEM
#endif 
 = {61155,57723,54484,51427,48539,45815,43243,40816,38525,36364,34323,32396};
//*********************************
#define RUN		0xB0
#define LIST	0xB1
#define NEW		0xB2
#define LOAD	0xB3

#define SAVE	0xB5
#define MEM		0xB6
#define CLS		0xB7
#define CLEAR	0xB8
#define BEEP	0xB9
#define HELP	0xBA

#define LOCATE	0xC0
#define OUT		0xC1

#define PRINT	0xE0
#define INPUT	0xE1
#define IF		0xE2
#define FOR		0xE4
#define NEXT	0xE5
#define GOTO	0xE7
#define GOSUB	0xE8
#define RETURN	0xE9
#define STOP	0xEA
#define CLR		0xEB
#define REM		0xEC
#define LET		0xED
#define PAUSE	0xEE

const struct commands table_cmd[]
#ifdef AVR
PROGMEM
#endif 
 = {
  "PRINT", PRINT,{MLTVAL,NONEVAL,END},//str,exp,mod
  "INPUT", INPUT,{VARVAL,END},//var
  "IF", IF,{EXPVAL,THEN,CMDVAL,END},//required THEN
  "GOTO", GOTO,{EXPVAL,END},
  "FOR", FOR,{VARVAL,'=',EXPVAL,TO,EXPVAL,END},
  "NEXT", NEXT,{VARVAL,END},
  "GOSUB", GOSUB,{EXPVAL,END},
  "RETURN", RETURN,"",//none
  "REM", REM,{ALLVAL,END},//any syms
  "LET", LET,{VARVAL,'=',EXPVAL,END},
  "LOCATE", LOCATE,{EXPVAL,',',EXPVAL,END},
  "BEEP", BEEP,{EXPVAL,',',EXPVAL,END},
  "OUT", OUT,{NUMVAL,',',EXPVAL,END},
  "STOP", STOP,"",
  "PAUSE", PAUSE,{EXPVAL,END},

  "HELP", HELP,{NONEVAL,END},
  "RUN", RUN,"",
  "MEM", MEM,"",
  "CLS", CLS,"",
  "CLEAR", CLEAR,"",
  "LIST", LIST,{NOPVAL,END},//optional num
  "NEW", NEW,"",
  "LOAD", LOAD,{STRVAL,END},
  "SAVE", SAVE,{STRVAL,END},
  "", END,""
};
//******************************************
#define IN		0xF1
#define INKEY	0xF2
#define RND		0xF3

const struct commands table_fn[]
#ifdef AVR
PROGMEM
#endif 
 = {
  "IN", IN,{NUMVAL,END},
  "INKEY", INKEY,"",
  "RND", RND,"",
  "", END, ""
};
//**************************
struct keys { //input keys
  uint8_t key_code;
  uint8_t mode;//bits: 0-console,1-programm,2-modifier,...4-alt,
  uint8_t tok;
};
//**************************
#define CONMODE 1
//console
#define PRGMODE 2
//program
#define FNMODE 4
//function
#define ALTKEY 0x10


const struct keys table_key[]
#ifdef AVR
PROGMEM
#endif 
 = {
  'b',0			| PRGMODE, BEEP,
  'c',0			| CONMODE | PRGMODE, CLS,
  'c',ALTKEY 	| CONMODE, CLEAR,
  'f',0			| PRGMODE, FOR,
  'g',ALTKEY	| PRGMODE, GOSUB,
  'g',0			| PRGMODE, GOTO,
  'h',0			| CONMODE, HELP,
  'i',0			| PRGMODE, IF,
  'i',ALTKEY	| PRGMODE, INPUT,
  'l',0			| PRGMODE, LET,
  'l',ALTKEY	| PRGMODE, LOCATE,
  'l',0			| CONMODE, LIST,
  'l',ALTKEY	| CONMODE | PRGMODE, LOAD,
  'm',0			| CONMODE | PRGMODE, MEM,
  'n',0			| CONMODE, NEW,
  'n',0			| PRGMODE, NEXT,
  'o',0			| PRGMODE, OUT,
  'p',ALTKEY	| PRGMODE, PAUSE,
  'p',0			| PRGMODE, PRINT,
  'r',0			| PRGMODE, REM,
  'r',ALTKEY	| PRGMODE, RETURN,
  'r',0			| CONMODE, RUN,
  's',0			| CONMODE | PRGMODE, SAVE,
  's',ALTKEY	| PRGMODE, STOP,

  'i',0			| FNMODE, IN,
  'i',ALTKEY	| FNMODE, INKEY,
  'r',0			| FNMODE, RND,
  ' ',0, END
};

//**************************
enum {
	EOK=0,
	EERROR=1,
	EALLOC=2,
	EGSOVF=3,
	ERETWOG=4,
	ELOPSOVF=5,
	ENEXTWOFOR=6,
	EGOTONOWHERE=7,
	EINTERUPT=8,
	EMORE=9,
	ECMDS=10,
	EFUNCS=11,
	ESTOP=12,
	EALT=13,
};

const uint8_t table_errc[][20]
#ifdef AVR
PROGMEM
#endif 
 = {
  "Ok",
  "E:Error",
  "E:alloc mem",
  "E:GOSUB stack ovfl",
  "E:RETURN w/o GOSUB",
  "E:FOR limits ovf",
  "E:NEXT w/o FOR",
  "E:Goto to nowhere",
  "Interrupted",
  "More?",
  "Commands",
  "Functions",
  "STOP",
  "<Alt>",
};
//**********************************************************************************************

struct loopvar {
	struct plines *line_begin;
	int var_to;
};
struct plines {
	struct plines *next;
	uint8_t lnum;
	uint8_t line[];
};

volat struct loopvar LoopVar[LMAX];
volat int Vars[VMAX]; /* program variable value */
volat struct plines *SubStack[SMAX]; /* subroutine stack */
volat uint8_t SubStackP; /* subroutine stack pointer */
volat uint8_t CmdInp[BMAX+1]; /* command input buffer */
//***********************************************
volat struct plines *FirstPrgLine=NULL;//always on first line
volat struct plines *PrgLineP=NULL;//float poiner
volat uint8_t CLine;
//*****************************************
//volatile int gi;
volat uint8_t pmode,cursmode;//console...program, cursor mode
volat uint8_t keymode;//0-alt
volat uint8_t *gp;
volat uint8_t xt,yt,gy,pkey; 

volat uint8_t PrgSps[MAXPRGSZ] = {0,};//Program memory 

uint8_t ReplaceChar(uint8_t *str ,uint8_t fchar, uint8_t rchar);
uint8_t LoadPrgLine(uint8_t lnum);
void PrepToSavPrgLine(struct plines *line);

