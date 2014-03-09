/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

void serror(uint8_t ec,uint8_t mode)
{
	if(!mode){
		gotocmdline();
		lputint(CLine);
	}
	lputchar(':');
	lFputs(table_errc[ec]);
//    lputchar('\n');
}
//**********************************************************
int GetUsedMem(void)
{
	register int sz=0;
	register uint8_t bsz;
	register uint8_t *lp;

	lp = PrgSps;
	while(*lp){
		bsz = (*lp & 0x7F);
		if(!(*lp & 0x80))//not free block
			sz +=bsz;
		lp +=bsz;
	}
	return sz;
}

void *lmalloc(uint8_t size)//max size 126 bytes
{
	register int sz=0;
	register uint8_t bsz;
	register uint8_t *lp;

	if(size > 126 || size == 0)
		return NULL;
	lp = PrgSps;
	while(*lp){
		bsz = (*lp & 0x7F);
		sz +=bsz;
//printf("\n+MEM USE:%d,lp=%X:%X  req:%d\n",sz,lp,*lp,size);
		if(*lp & 0x80){//free block
			if((bsz-1) >= size || bsz == 0){
				*lp = bsz;//mark as used
				return(lp+1);
			}
			else{
				if(*(lp+bsz) & 0x80){//next block is free
					if(*(lp+bsz) == 0x80)//end of heap
						break;
				}
			}
		}
		lp +=bsz;
	}
//printf("\n1MEM USE:%d,lp=%X:%X  req:%d\n",sz,lp,*lp,size);
	if((sz+size) < MAXPRGSZ){
		*lp= size+1;//mark as used
		*(lp+size+1) = 0; //mark end of heap
		return(lp+1);
	}
	return NULL;
}

void lfree(void *pnt)
{
	register uint8_t *lp;

	lp = (pnt-1);
	if(pnt == NULL) return;
	if(*(lp+*lp) == 0)//heap end
		*lp = 0;//move heap
	else
		*lp= *lp | 0x80;//mark as free
//printf("\nMEM FREE:lp=%X:%X\n",lp,*lp);
}

//**********************************************************
struct plines *GetPrgLine(uint8_t bynline, uint8_t mode)//line, mode: 0-def,1- =>
{
	register struct plines *lp;

    lp = FirstPrgLine;
    while(lp){
    	if(lp->lnum >= bynline){
    		if(lp->lnum > bynline && !mode)
				lp = NULL;
    	    break;
		}
	lp = lp->next;
    };
	return lp;
}

uint8_t DelPrgLine(uint8_t lnum)
{
	register struct plines *lp, *cp;

	lp = GetPrgLine(lnum,0);
	if(lp){
		if(lp == PrgLineP)
			PrgLineP = NULL;
		if(lp == FirstPrgLine)
			FirstPrgLine = FirstPrgLine->next;
		else{
			cp = FirstPrgLine;
			while(cp){
    			if(cp->next == lp){
					cp->next = lp->next;
					break;
				}
				cp = cp->next;
    		}
		}
		lfree(lp);
		lp = NULL;
		return 1;
	}
	else
		return 0;
}

/*
struct plines *LastPrgLine()
{
	register struct plines *lp;

    lp = FirstPrgLine;
    while(lp){
	if(!lp->next)
	    break;
	lp = lp->next;
    };
    return lp;
}
*/
struct plines *AddEmpLine(uint8_t lsize)
{
	register struct plines *lp;

//printf("\nADD:%d %d\n",lsize,sizeof(struct plines));
    lsize += sizeof(struct plines);
    if(lp = lmalloc(lsize))
    {
	lp->next = NULL;
	return lp;
    }
    serror(EALLOC,0);
    return NULL;
}

struct plines *AddPrgLine(uint8_t *pline, uint8_t lnum)
{
	register uint8_t la;
	register struct plines *lp,*lp1,*lp2;

    la = strlen((const char *)pline)+1;//+null
	lp = AddEmpLine(la);
	if(!lp)
	    return NULL;
    if(!FirstPrgLine)//first init
	FirstPrgLine = lp;
    else {
//get line with lnumber (=|<) lnum
	lp1 = FirstPrgLine;
	lp2 = NULL;//prev -> parent
	while(lp1){
	    if(lp1->lnum == lnum){
		lp->next = lp1->next;
	        lfree(lp1);
			lp1 = NULL;
		break;
	    }
	    if(lp1->lnum > lnum){
		lp->next = lp1;
		break;
	    }
	    lp2 = lp1;
	    lp1 = lp1->next;
	};
//--------------------------
	if(!lp2)//prev==0 - will be the first
	    FirstPrgLine = lp;
	else//insert  lp2< lp >lp1
	    lp2->next = lp;
    }
    
    lp->lnum = lnum;
    strcpy((char *)(lp->line),(const char *)pline);
    return(lp);
}
//***************************************

void ResetEnv(void)//clear
{
	register uint8_t la=0;

	ClearScreen();
	SubStackP = 0; //reset sub stack pointer
	for(;la<VMAX;Vars[la++]=0);//(re)set vars
}

void FreePrg(void)//new
{

	ResetEnv();
	PrgSps[0]=0;
	FirstPrgLine = NULL;
}
//***********************************************

uint8_t *findchar(uint8_t *str ,uint8_t ch)
{
    register uint8_t *lp;

    for(lp=str;*lp;lp++){
	if(*lp == ch)
	    return lp;
    }
    return 0;
}

uint8_t ReplaceChar(uint8_t *str ,uint8_t fchar, uint8_t rchar)
{
    register uint8_t *lp,lc=0;

    for(lp=str;*lp;lp++){
		if(*lp == fchar){
			*lp = rchar;
		    return lc;
		}
		lc++;
    }
    return 0;
}

uint8_t FromHexStr(uint8_t *str)//2 hex str bytes to 1
{
    register uint8_t t,t1;

    t = *str;
    t1 = *++str;
    if(t<'0' || t>'F' ) return 0;
    (t>'9') ? (t-='7'): (t-='0');
    if(t1<'0' || t1>'F' ){
	t1 = t;
	t = 0;
    }
    else
	(t1>'9') ? (t1-='7') : (t1-='0');
    return ((t<<4)|t1);
}

void ToHexSTR(uint8_t *str)//1 byte to 2 nex str
{
    register uint8_t t,t1;

    t = str[0];
    t1 = t>>4;
	t = t-(t1<<4);
	(t1>9) ? (t1+='7') : (t1+='0');
	(t>9) ? (t+='7') : (t+='0');
	str[0] = t1;
	str[1] = t;
	str[2] = 0;
}

uint8_t SymIsExp(uint8_t sym)
{
	register uint8_t li=0;

#ifdef AVR
	while(pgm_read_byte(&(exp_syms[li]))){
		if(pgm_read_byte(&(exp_syms[li]))==sym)
#else
	while(exp_syms[li]){
		if(exp_syms[li]==sym)
#endif
			return 1;
		li++;
	}
	if(SYMISVAR(sym))
		return 2;
	if(SYMISNUM(sym))
		return 3;
	if(SYMISFN(sym))
		return 4;

	return 0;
}
//***********************************************************
uint8_t KeyToCmd(uint8_t key)
{
	register uint8_t li,kst,ch;

	kst = ((keymode&ALT_FLG)<<4);
	li=0;
#ifdef AVR
	while(ch=pgm_read_byte(&(table_key[li].tok))){
		if(pgm_read_byte(&(table_key[li].key_code)) == key){
			if((pgm_read_byte(&(table_key[li].mode)) & pmode) &&
							(pgm_read_byte(&(table_key[li].mode))&0xf0) == kst)
				return pgm_read_byte(&(table_key[li].tok));
#else
	while(ch=table_key[li].tok){
		if(table_key[li].key_code == key){
			if((table_key[li].mode & pmode) && (table_key[li].mode&0xf0) == kst)
				return table_key[li].tok;
#endif
		}
		li++;
	}
	return 0; 
}
//***********************************************************
void print_ssym(uint8_t s)
{
    register uint8_t li,ch;

	li=0;
#ifdef AVR
	while(ch = pgm_read_byte(&(table_ssymb[li].tok))){
#else
	while(ch = table_ssymb[li].tok){
#endif
		if(s == ch){
			lFputs(table_ssymb[li].text);
			break;
		}
		li++;
	}
}

uint8_t print_code(uint8_t *line, uint8_t curspos)
{
    register uint8_t li,lc,lj,ch,ac=0;

	li=0;

	if(!line){
		curspos =255;
		lputint(PrgLineP->lnum);
//		lputchar(' ');
		line = PrgLineP->line;
	}

	while(lc= line[li]){

		if(SYMISCMD(lc)& !ac){//command code
			lputchar(' ');
//------------------------
			lj=0;
#ifdef AVR
			while(ch=pgm_read_byte(&(table_cmd[lj].tok))){
#else
			while(ch=table_cmd[lj].tok){
#endif
				if(ch == lc){
					lFputs(table_cmd[lj].text);
					break;
				}
				lj++;
			}
//------------------------
			lputchar(' ');
			if(ch==REM) ac =1;
	    }else if(SYMISFN(lc)& !ac){//fn code
//			lputchar(' ');
//------------------------
			lj=0;
#ifdef AVR
			while(ch=pgm_read_byte(&(table_fn[lj].tok))){
#else
			while(ch=table_fn[lj].tok){
#endif
				if(ch == lc){
					lFputs(table_fn[lj].text);
					break;
				}
				lj++;
			}
//------------------------
	    }
		else if (SYMISEXTSYM(lc)){
			lputchar(' ');
		    print_ssym(lc);
			lputchar(' ');
		}
		else{
//		if(SYMISCHAR(lc)){//expression or text
		    lputchar(lc);
			if(lc == '"')
				if(ac)
					ac=0;
				else
					ac=1;
		}
		if(curspos == li)
			PutCursor();
	    li++;
	}
	if(curspos == li)
		PutCursor();
	return lj;
}

void PrintCmdLine(uint8_t curspos)
{
    register uint8_t ly;

	ly = yt;
	gotocmdline();
	lputchar('>');

	if(CLine){
		lputint(CLine);
		lputchar(' ');
	}

	print_code(CmdInp, curspos);
/*uart_putchar('\r');
uart_putchar('+');
uart_putchar(':');
uart_put_dec(ly);
uart_putchar(':');
uart_put_dec(yt);
uart_putchar('\r');
*/
	if(ly != yt){
		yt = ly;
	}
}
//*******************************************************
uint8_t LoadPrgLine(uint8_t lnum)//line num
{
    register uint8_t lc;

	if(!lnum){
		lnum = FromHexStr(CmdInp);
		lc = 2;
	}
	else
		lc = 0;
    if(lnum){
	if(!(PrgLineP = AddPrgLine(CmdInp+lc,lnum)))
//	if(!AddPrgLine(CmdInp+lc,lnum))
	    return(1);
    }
    return(0);
}

void PrepToSavPrgLine(struct plines *line)
{
//    register uint8_t lc;

	CmdInp[0] = line->lnum;
	ToHexSTR(CmdInp);
	strcpy((char *)(CmdInp+2),(const char *)(line->line));
}

//*******************************************************
uint8_t CheckCmd(uint8_t *cline)
{
    register uint8_t lc,qc,ch;
    register uint8_t *lpc=NULL,*bexpr=NULL;

up:
	cursmode = NONEVAL;
	lc=cline[0];
	if(SYMISCMD(lc))
//**********************************
//		lpc = look_templ(lc);
//uint8_t *look_templ(uint8_t s)
{
	qc=0;
#ifdef AVR
	while(ch=pgm_read_byte(&(table_cmd[qc].tok))){
#else
	while(ch=table_cmd[qc].tok){
#endif
      if(ch == lc){
		lpc = (uint8_t *)(table_cmd[qc].templ);
		break;
	  }
	qc++;
  }
//  return 0;
}
//******************************
	else
		return(ERRVAL);
	qc=0;
	cline++;
#ifdef AVR
	ch = pgm_read_byte(lpc);
#else
	ch = *lpc;
#endif
	lc = ch;
	while(*cline && ch){
//printf("\n--%c\n",*cline);
		if(ch == MLTVAL){//str,mod-fn,exp(num,var){
			if(lc != STRVAL && lc != CMDVAL && lc != EXPVAL){
				if(*cline == '"')//str
					lc = STRVAL;
				else if(SYMISCMD(*cline))//mod or fn
					lc = CMDVAL;
				else//expression 
					lc = EXPVAL;
			}
		} else	lc = ch;

		switch(lc){
		case CMDVAL://command
			if(!SYMISCMD(*cline))
				goto err;
			goto up;
//			lpc++;
			break;
		case ALLVAL://any sym
				cursmode = OKVAL;
				return(STRVAL);
			break;
		case VARVAL://variable
			if(!SYMISVAR(*cline))
				goto err;
			lpc++;
			cursmode = okVAL;
			break;
		case EXPVAL://expression
			if(SymIsExp(*cline)){
				if(*cline == '(')
					qc++;
				else if(*cline == ')')
					qc--;
				if(!bexpr)
					bexpr = cline;

				if(SYMISVAR(*cline) && SYMISVAR(*(cline-1)))
					goto err;

				if(!qc && *cline != '='){
					cursmode = okVAL;
//					lc = okVAL;
				}
				else
					cursmode = ch;
//printf("\n++%c-%c\n",cursmode,*cline);
			}
			else{
//printf("\n+2+%c-%c\n",cursmode,*cline);
				if(bexpr && cursmode == okVAL){//was exp and parentheses is closed
					lpc++;
					cline--;
					bexpr=NULL;
				}
				else
					goto err;
			}
			break;
		case NOPVAL://num optional
			cursmode = okVAL;
		case NUMVAL://number
			if(SYMISNUM(*cline)){
				if(!bexpr)
					bexpr = cline;
				cursmode = okVAL;
			}
			else
				if(bexpr){
					lpc++;
					cline--;
					bexpr=NULL;
				}
				else
					goto err;
/*					if(atoi(bexpr)>SIZEOFVAR())
						return(ERRVAL);
*/
			break;
		case STRVAL://string
			if(*cline == '"'){
				if(qc) qc=0;
				else qc=1;
				if(!qc){
					lpc++;
					cursmode = okVAL;
				}
			}
			else
				if(!qc)//open quote
					goto err;
			break;

		default:
			*cline = ch;
			lpc++;
			cursmode = ch;
		}
		cline++;
#ifdef AVR
		ch = pgm_read_byte(lpc);
#else
		ch = *lpc;
#endif
	}
err:

//printf("\n**-%c-%c-%c-%c\n",*cline,ch,cursmode,lc);

	if(ch != MLTVAL)
		lc = ch;

	if(*cline == END){
		if(ch == END){
			if(cursmode != ERRVAL)
				cursmode = OKVAL;
		}
		else
			if(cursmode == okVAL){
				if(*(lpc+1) == END)
					cursmode = OKVAL;
				else
					cursmode = okVAL;
			}
			else
				cursmode = lc;
	}
	else
		cursmode = ERRVAL;

	return(lc);
}

//**************************************************************
uint8_t lgets(uint8_t *str)
{
	register uint8_t ch,t;
	register uint8_t *lp;

	lp =str;
	*str=END;

	gotocmdline();
	lputchar(':');

	while(1){
		ch=lgetchar();
		if(ch == BREAK_KEY){
			lp = str+1;
			*str=BREAK_KEY;
			break;
		}
		if(ch == KBD_ENTER) break;
		else if(ch == KBD_BS){
			if(str > lp) str--;
		}
		else if(t=SymIsExp(ch)){
			*str = ch;
			str++;

		}
		*str = 0;
		gotocmdline();
		lputchar(':');
		lputs((char *)lp);
	};
	if(str == lp){
		*str='0';
		str++;
		*str = 0;
	}
	lputchar('\n');
	return(ch);
}

