/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

#define STOPPROG(x) {PrgLineP = NULL;return(x);}
uint8_t GoLine(void)
{
	register uint8_t li;
	register uint8_t *lp;
	register uint16_t bi;
//printf("\n#%d:%s\n",CLine,CmdInp);
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
	    gp+=2;
	    Vars[TOVAR(CmdInp[1])] = ExpPars1();
	    return(0);
	case LOCATE:
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
	case INPUT:
	    li = *gp;
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
	    if(CmdInp[1] == '"'){
		ReplaceChar(gp+1 ,'"', 0);
		lputs((char *)(gp+1));
	    }
	    else{
		lputint(ExpPars1());
	    }
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
	}
	return(0);
}
