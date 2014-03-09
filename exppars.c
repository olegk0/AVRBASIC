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

    switch(*gp++){
    case '=':
	return(o == ExpPars1());
//	break ;
    case NESIGN:
	return(o != ExpPars1());
    default:
	gp--;
	return(o);
    }
}

int ExpPars2(void)
{
    int o = ExpPars3();

    switch(*gp++){
    case '<':
	return(o < ExpPars2());
//	break;
    case '>':
	return(o > ExpPars2());
    default:
	gp--;
	return(o);
    }
}

int ExpPars3(void)
{
    int o = ExpPars4();

    switch(*gp++){
    case LESIGN:
	return(o <= ExpPars3());
//	break ;
    case GESIGN:
	return(o >= ExpPars3());
    default:
	gp--;
	return o;
    }
}

int ExpPars4(void)
{
    int o = ExpPars5();

    switch(*gp++){
    case '+':
	return(o+ExpPars4());
//	break;
    case '-':
	return(o-ExpPars4());
    default:
	gp--;
	return(o);
    }
}

int ExpPars5(void)
{
    int o = ExpPars6();

    switch(*gp++){
    case '*':
	return(o*ExpPars5());
//	break ;
    case '/':
	return(o/ExpPars5());
    default:
	gp--;
	return o;
    }
}

int ExpPars6(void)
{
    int o;
//printf("\n*6*%d\n",*gp);

	if(SYMISFN(*gp)){
		switch(*gp){
		case IN:
			gp++;
			if(*gp == '('){
				gp++;
				o=ExpPars1();
				gp++;
				return in_port(o);
			}
			else
				return 0;
			break;
		case INKEY:
			gp++;
			o = pkey;
			pkey = 0;
			return 0;
			break;
		case RND:
			gp++;
			return rand();
			break;
		default:
			return 0;
		}
	}
	else if(*gp == '-'){
		gp++;
		return -ExpPars6();
	}
	else if(SYMISNUM(*gp))
		return strtol((const char *)gp, &gp, 0);
	else if(*gp == '('){
		gp++;
		o=ExpPars1();
		gp++;
		return o;
	}
	else
		return Vars[TOVAR(*gp++)];
/*
    return((*gp == '-') ? (gp++, -ExpPars6()):
	    SYMISNUM(*gp) ? strtol((const char *)gp, &gp, 0):
	    (*gp == '(') ? (gp++, o=ExpPars1(),gp++,o):
	    Vars[TOVAR(*gp++)]);
*/
}

