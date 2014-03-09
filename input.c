/* AVRBASIC (c) 2014 olegvedi@gmail.com 
*/

	CLine = 0;
	cf=0;
//	ResetEnv();//for test
	CmdInp[0] = 0;
	pmode = CONMODE;
	cursmode = NONEVAL;
	PrintCmdLine(0);
	do{
	    cp = lgetchar();
//printf("\n%2.2X\n",cp);
		switch(cp){
		case BREAK_KEY:
			CmdInp[0] = END;
			cf = 0;
			CLine = 0;
			cursmode = NONEVAL;
			break;
		case KBD_BS:
			if(cf>0)
				cf--;
			else if(CLine && cursmode != ERRVAL){
				CLine = 0;
				pmode = CONMODE;
			}
			if(cursmode == ERRVAL || cf==0)
				cursmode = NONEVAL;
			CmdInp[cf] = END;
//			cp='1';
			break;
		case KBD_ENTER:
			if(cursmode == NOPVAL || cursmode == OKVAL || (CLine && !CmdInp[0]))
				goto end;
			else
				cursmode = ERRVAL;
			break;
//		case SPACE_KEY:
//			break;
		default:
			if(cursmode != ERRVAL && ((keymode&CL_FLG && (cursmode == ALLVAL || cursmode == STRVAL)) || !(keymode&CL_FLG))){
    			if(cf<(BMAX-2)){
					if(cf>0){
						if(cp == '='){
							if(CmdInp[cf-1] == '>'){//>=
								cp = GESIGN;
								cf--;
							}
							else if(CmdInp[cf-1] == '<'){//<=
								cp = LESIGN;
								cf--;
							}
						}else if(cp == '>')//<>
							if(CmdInp[cf-1] == '<'){
								cp = NESIGN;
								cf--;
							}
					}
					CmdInp[cf] = cp;
 					cf++;
					CmdInp[cf] = END;
				}
			}
		}

		if(CmdInp[0]){
			if(!CLine && SYMISNUM(CmdInp[0])){//line num detected

				if(cp == ' '){
					if(atoi((char *)CmdInp)){
						CLine = atoi((char *)CmdInp);
						cursmode = CMDVAL;
	 					cf=0;
						CmdInp[0] = END;
						pmode = PRGMODE;
					}
				}
				else if(SYMISNUM(CmdInp[cf-1]) && atoi((char *)CmdInp)<=LNMAX)
					cursmode = NUMVAL;
				else
					cursmode = ERRVAL;
			}
			else{ //cmd
				if(CmdInp[1]==END || cursmode==CMDVAL){//one sym or cmd
/*					if(cp==EDIT_KEY){
						PrgLineP = GetPrgLine(CLine, 0);
						if(PrgLineP){
							strcpy((char *)(CmdInp),(const char *)(PrgLineP->line));
							cf = strlen(CmdInp)-1;
						}
					}
					else*/
					{
						cp = KeyToCmd(cp);//<+keymode
						if(cp)
							CmdInp[cf-1] = cp;
						else
							cursmode = ERRVAL;
					}
				} else
					if(cursmode==MLTVAL || cursmode==EXPVAL){//fn ?
						pmode = FNMODE;
						cp = KeyToCmd(cp);//<+keymode
//**********************************************************
						if(cp){//TODO  yet simple scheme without check 
							CmdInp[cf-1] = cp;
							gp =NULL;
							gy=0;
#ifdef AVR
							while(ct=pgm_read_byte(&(table_fn[gy].tok))){
#else
							while(ct=table_fn[gy].tok){
#endif
								if(ct == cp){
									gp = (uint8_t *)(table_fn[gy].templ);
									break;
	  							}
								gy++;
  							}
#ifdef AVR
							cp = pgm_read_byte(gp);
#else
							cp = *gp;
#endif
							if(cp){//fn with argument (for only one argument)
								CmdInp[cf] = '(';
								cf++;
								CmdInp[cf] = END;
							}
						}
//*****************************************************************
						pmode = PRGMODE;
					}
//printf("\n||--%c-%d-\n",cp,pmode);
				cp = CheckCmd(CmdInp);
				if(cursmode == ERRVAL || (CmdInp[cf-1] == ' ' && cp != STRVAL)){
					cf--;
					CmdInp[cf] = END;
					if(cf == 0) cursmode = NONEVAL;
				}
			}
		}
		if(cursmode == ERRVAL)
			Beep(50,0);
		PrintCmdLine(cf);
	}
	while(1);
end:

