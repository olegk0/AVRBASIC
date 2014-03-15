/*
AVRBASIC (c) 2014 olegvedi@gmail.com 

DDS-BASIC Interpreter (Version 1.00)
Copyright (c) 1990, Landon Curt Noll & Larry Bassel.
All Rights Reserved.  Permission for personal, educational or non-profit use is
granted provided this this copyright and notice are included in its entirety
and remains unaltered.  All other uses must receive prior permission in writing
from both Landon Curt Noll and Larry Bassel.
*/


int ExpPars1(void);
int ExpPars2(void);
int ExpPars3(void);
int ExpPars4(void);
int ExpPars5(void);
int ExpPars6(void);

/* recursive descent parser for arithmetic/logical expressions */
int ExpPars1(void)
{
    int o = ExpPars2();

    switch(*Gp++){
    case '=':
	return(o == ExpPars1());
//	break ;
    case NESIGN:
	return(o != ExpPars1());
    default:
	Gp--;
	return(o);
    }
}

int ExpPars2(void)
{
    int o = ExpPars3();

    switch(*Gp++){
    case '<':
	return(o < ExpPars2());
//	break;
    case '>':
	return(o > ExpPars2());
    default:
	Gp--;
	return(o);
    }
}

int ExpPars3(void)
{
    int o = ExpPars4();

    switch(*Gp++){
    case LESIGN:
	return(o <= ExpPars3());
//	break ;
    case GESIGN:
	return(o >= ExpPars3());
    default:
	Gp--;
	return o;
    }
}

int ExpPars4(void)
{
    int o = ExpPars5();

    switch(*Gp++){
    case '+':
	return(o+ExpPars4());
//	break;
    case '-':
	return(o-ExpPars4());
    default:
	Gp--;
	return(o);
    }
}

int ExpPars5(void)
{
    int o = ExpPars6();

    switch(*Gp++){
    case '*':
	return(o*ExpPars5());
//	break ;
    case '/':
	return(o/ExpPars5());
    default:
	Gp--;
	return o;
    }
}

int ExpPars6(void)
{
    int o;
	uint8_t *lp;
	unsigned int t;
//printf("\n*6*%s\n",Gp);

	if(SYMISFN(*Gp)){
		switch(*Gp){
		case IN:
			Gp++;
			if(*Gp == '('){
				Gp++;
				o=ExpPars1();
				Gp++;
				return in_port(o);
			}
			else
				return 0;
			break;
		case INKEY:
			Gp++;
			o = pkey;
			pkey = 0;
			return o;
			break;
		case RND:
			Gp++;
			if(*Gp == '('){
				Gp++;
				o = ExpPars1();
				if(o>0)
					o = rand()/(SIZEOFVAR()/o);
				else
					o=0;
				Gp++;
				return o;
			}
			else
				return 0;
			break;
		default:
			return 0;
		}
	}
	else if(*Gp == '-'){
		Gp++;
		return -ExpPars6();
	}
	else if(SYMISNUM(*Gp))
		return strtol((const char *)Gp, (char **)(&Gp), 0);
	else if(*Gp == '('){
		Gp++;
		o=ExpPars1();
		Gp++;
		return o;
	}
	else if(SYMISVAR(*Gp)){
//printf("\n*6*%s\n",Gp);
		o = TOVAR(*Gp);
		t = (unsigned int)Vars[o];//var value as pointer
		Gp++;
		if(*Gp == '(' && t > (unsigned int)LlP){//array and pointer to PrgSps
			Gp++;
			o = ExpPars1();//index
			Gp++;
			lp = (uint8_t *)t;
//printf("\n*%p %u %u\n",lp,(unsigned int)(*(((unsigned int *)lp + o))),o);
			if(*(lp-2) > (2+(uint8_t)o)){ //mem block size > index
				switch(*(lp-1)){//type of array
				case SIGNEDBYTE:
					return *(((char *)lp + o));
					break;
				case UNSIGNEDBYTE:
					return *(((uint8_t *)lp + o));
					break;
				default:
//				case SIGNEDWORD:
					return *(((int *)lp + o));
					break;
/*				case UNSIGNEDWORD:
					return *(((unsigned int *)lp + o));
					break;
*/
				}
			}
//			return Vars[o];
		}
//printf("\n+%c %d %d\n",t,TOVAR(t),Vars[TOVAR(t)]);
		return Vars[o];
	}
	else
		return 0;
}

