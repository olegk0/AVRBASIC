/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

#define STOPPROG(x) {PrgLineP = NULL;return(x);}
uint8_t GoLine(void)
{
	register uint8_t li;
	register uint8_t *lp;
	register uint16_t bi;
//printf("\n#%d:%s\n",CLine,CmdInp);
//printf("\n*lnum=%d A=%d\n",CLine,Vars[0]);
	gp = CmdInp + 1;
	switch(CmdInp[0]){
	case STOP:
		STOPPROG(ESTOP);
	case BEEP:
		lp = findchar(CmdInp, ',');
		if(lp)
		{
			*lp = 0;
		    bi = ExpPars1();
			gp=lp+1;
			Beep(bi,ExpPars1());
		}
	    return(0);
	case LET:
		lp = findchar(CmdInp, '=');
		li = CmdInp[1];
		if(*(lp-1) == ')'){
			*lp = 0;
			gp = CmdInp+2;// '('
		    li += ExpPars1();
			if(li>'Z')
				li = CmdInp[1];
		}
		gp = lp+1;
	    Vars[TOVAR(li)] = ExpPars1();
	    return(0);
	case AT:
		lp = findchar(CmdInp, ',');
		if(lp)
		{
			*lp = 0;
		    li = ExpPars1();//x
			if(li< LCDTWIDTH)
				xt=li*LCDSYMWIDTH;
			else
				xt=(LCDTWIDTH-1)*LCDSYMWIDTH;
			gp=lp+1;
		    li = ExpPars1();//y
			if(li< LCDTHEIGHT)
				yt=li;
			else
				yt=LCDTHEIGHT-1;
#ifdef AVR
			st7565_command(CMD_SET_DISP_START_LINE | ((LCDTHEIGHT-1)*8)+8);
#endif
		}
	    return(0);
	case OUT:
		lp = findchar(CmdInp, ',');
		if(lp)
		{
			*lp = 0;
		    li = ExpPars1();//port
			gp=lp+1;
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
		li = CmdInp[1];
		if(CmdInp[2] == '('){
			gp = CmdInp+2;// '('
		    li += ExpPars1();
			if(li>'Z')
				li = CmdInp[1];
		}
		lgets(CmdInp);
		if(CmdInp[0]==BREAK_KEY)
			STOPPROG(EINTERUPT);
		gp=CmdInp;
	    Vars[TOVAR(li)] = ExpPars1();
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
		li = strlen(CmdInp);
	    if(CmdInp[1] == '"'){
		ReplaceChar(gp+1 ,'"', 0);
		lputs((char *)(gp+1));
	    }
	    else{
		lputint(ExpPars1());
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
	case FOR : /* "FOR" */
	    lp = findchar(CmdInp, TO);
	    if(lp){
			*lp = 0; /* "TO" */
			gp = CmdInp+ 3;
			li = TOVAR(CmdInp[1]);
			if(li>LMAX)
				STOPPROG(ELOPSOVF);
			Vars[li] = ExpPars1();
			gp = lp + 1;
			LoopVar[li].var_to = ExpPars1();
			LoopVar[li].line_begin = PrgLineP->next;
	    }else
			STOPPROG(EERROR);
	    return(0);
	case NEXT: /* "NEXT" */
	    li = TOVAR(CmdInp[1]);
	    if(++Vars[li] <= LoopVar[li].var_to){
			PrgLineP = LoopVar[li].line_begin;
			return(1);
	    }
		break;
	}
	return(0);
}
