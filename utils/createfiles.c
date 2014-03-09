#include <stdio.h>

main()
{
FILE *f;
char fn[] ={'0','9','a','z',0},lfn[] ={0,0},ch;

unsigned char i=0;
int j;

    while(fn[i]){
	ch = fn[i];
	while(ch <= fn[i+1]){
	    lfn[0] = ch;
            f = fopen(lfn,"wb");
	    for(j=0;j< 512*4;j++)
	    fprintf(f,"0");
            fclose(f);
            ch++;
        }
        i=i+2;
    }
}