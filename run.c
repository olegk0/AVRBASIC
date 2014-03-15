/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

#define STOPPROG(x) {PrgLineP = NULL;return(x);}
uint8_t GoLine(void)
{
	register uint8_t li;
	register uint8_t *lp;
	register unsigned int bi;
//printf("\n#%d:%s\n",CLine,CmdInp);
//printf("\n*lnum=%d A=%d\n",CLine,Vars[0]);
	Gp = CmdInp + 1;
	switch(CmdInp[0]){
	case STOP:
		STOPPROG(ESTOP);
	case BEEP:
		lp = findchar(CmdInp, ',');
		if(lp)
		{
			*lp = 0;
		    bi = ExpPars1();
			Gp=lp+1;
			Beep(bi,ExpPars1());
		}
	    return(0);
	case DIM:
		Gp+=2;//(
	    li = ExpPars1();
		if(CmdInp[1] != SIGNEDBYTE && CmdInp[1] != UNSIGNEDBYTE)//type of array
			li = li << 1;//two byte on item
		if(li <= (MAXBMEMSIZE-1)){
/*			lp = Vars[TOVAR(CmdInp[1])];
			if(lp > FirstPrgLine)
				lfree(lp);*/
			lp = lmalloc(li+1,LlP);
			if(!lp)
				STOPPROG(EALLOC);
			*lp = CmdInp[1];//type of array
			lp++;
//printf("\n*%p %c %d %d %s\n",lp,*(lp-1),*(lp-2),li,CmdInp);
			li = CmdInp[2];//name of array
//			if(SYMISVAR(li))
				Vars[TOVAR(li)] = (unsigned int)lp;
		    return(0);
		}
		STOPPROG(EERROR);
	    return(0);
	case LET:
		lp = findchar(CmdInp, '=');
		Gp = lp+1;
		bi = ExpPars1();
//		if(!SYMISVAR(li))
		if(*(lp-1) == ')'){
			*lp = 0;
			Gp = CmdInp+2;// '('
			li = ExpPars1();//array index
			lp = (uint8_t *)Vars[TOVAR(CmdInp[1])];
//printf("\n+%p %c %d %d %d\n",lp,*(lp-1),*(lp-2),li,bi);
			if(*(lp-2) > (li+2)){//
				switch(*(lp-1)){//type of array
				case SIGNEDBYTE:
					*(((char *)lp + li)) = bi;
					break;
				case UNSIGNEDBYTE:
					*(((uint8_t *)lp + li)) = bi;
					break;
				default:
//				case SIGNEDWORD:
					*(((int *)lp + li)) = bi;
					break;
/*				case UNSIGNEDWORD:
					*(((unsigned int *)lp + li)) = bi;
					break;
*/
				}
			}
			else
				STOPPROG(EERROR);//TODO
		}
		else
		    Vars[TOVAR(CmdInp[1])] = bi;
	    return(0);
	case AT:
		lp = findchar(CmdInp, ',');
		if(lp)
		{
			*lp = 0;
		    li = ExpPars1();//x
			if(li< LCDTWIDTH)
#ifdef AVR
				xt=li*LCDSYMWIDTH;
			else
				xt=(LCDTWIDTH-1)*LCDSYMWIDTH;
#else
				xt=li;
			else
				xt=(LCDTWIDTH-1);
#endif
			Gp=lp+1;
		    li = ExpPars1();//y
			if(li< LCDTHEIGHT)
				yt=li;
			else
				yt=LCDTHEIGHT-1;
#ifdef AVR
			st7565_command(CMD_SET_DISP_START_LINE | ((LCDTHEIGHT-1)*8)+8);
#else
			printf( "%c[%d;%dH", 27, yt+1, xt+1 ); // установили курсор в позицию 
 			fflush( stdout ); 
#endif
		}
	    return(0);
	case OUT:
		lp = findchar(CmdInp, ',');
		if(lp)
		{
			*lp = 0;
		    li = ExpPars1();//port
			Gp=lp+1;
			out_port(li,ExpPars1());
		}
	    return(0);
	case REM:
	    return(0);
	case LOAD:
		FreePrg();
		ReplaceChar(CmdInp+2, '"', 0);
		loadprg((const char *)(CmdInp+2));
		PrgLineP = FirstPrgLine;
		return(1);
	case INPUT:
		lp = (uint8_t *)(Vars + TOVAR(CmdInp[1]));//pointer to var
		Gp++;//to '(' or 0
		if(*Gp == '(')
			li = ExpPars1();//index
		else
			li = 255;
		lgets(CmdInp);
		if(CmdInp[0]==BREAK_KEY)
			STOPPROG(EINTERUPT);
		Gp=CmdInp;
		bi = ExpPars1();

		if(li < MAXBMEMSIZE){
			lp = (uint8_t *)(*((unsigned int *)lp));
			if(*(lp-2) > (li+2)){//
				switch(*(lp-1)){//type of array
				case SIGNEDBYTE:
					*(((char *)lp + li)) = bi;
					break;
				case UNSIGNEDBYTE:
					*(((uint8_t *)lp + li)) = bi;
					break;
				default:
					*(((int *)lp + li)) = bi;
					break;
				}
			}
			else
				STOPPROG(EERROR);//TODO
		}
		else
		    *lp = (int)bi;
	    return(0);
	case IF:
	    lp = findchar(CmdInp, THEN);
	    if(lp){
		*lp = 0;

		if(ExpPars1()){
		    strcpy((char *)CmdInp,(const char *)(lp+1));
		    return(2);;
		}
	    }else
		STOPPROG(EERROR);
	    return(0);
	case PRINT:
		li = strlen((const char *)CmdInp);
		while(Gp && *Gp){
		    if(*Gp == '"'){
				Gp++;
				lp =findchar(Gp ,'"');
				*lp = 0;
				lputs((char *)(Gp));
				Gp = lp+1;
		    }
			else{
				if(*Gp == '$'){
					Gp++;
					lputchar(ExpPars1());	
			    }
				else
					lputint(ExpPars1());
		    }
//printf("\n++ %s\n",Gp);
			Gp =findchar(Gp ,',');
			if(Gp)
				Gp++;
		}

		if(CmdInp[li-1]!=';')
		    lputchar('\n');
	    return(0);
	case PAUSE:
		bi = ExpPars1();
	    delay_ms(bi);
	    return(0);
	case GOTO:
	    PrgLineP = GetPrgLine(ExpPars1(),0);
		if(!PrgLineP){
			STOPPROG(EGOTONOWHERE);
		}
	    return(1) ;
	case GOSUB:
	    SubStack[SubStackP] = PrgLineP->next;
	    SubStackP++;
	    if(SubStackP >= SMAX){
		STOPPROG(EGSOVF);
	    }
	    PrgLineP = GetPrgLine(ExpPars1(),0);
		if(!PrgLineP){
			STOPPROG(EGOTONOWHERE);
		}
	    return(1);
	case RETURN:
	    if(SubStackP < 1){
		STOPPROG(ERETWOG);
	    }
	    --SubStackP;
	    PrgLineP = SubStack[SubStackP];
	    return(1);
	case FOR :
	    lp = findchar(CmdInp, TO);
	    if(lp){
			*lp = 0;
			Gp = CmdInp+ 3;
			li = TOVAR(CmdInp[1]);
			if(li>LMAX)
				STOPPROG(ELOPSOVF);
			Vars[li] = ExpPars1();
			Gp = lp + 1;
			LoopVar[li].var_to = ExpPars1();
			LoopVar[li].line_begin = PrgLineP->next;
	    }else
			STOPPROG(EERROR);
	    return(0);
	case NEXT:
	    li = TOVAR(CmdInp[1]);
	    if(++Vars[li] <= LoopVar[li].var_to){
			PrgLineP = LoopVar[li].line_begin;
			return(1);
	    }
		break;
	}
	return(0);
}
