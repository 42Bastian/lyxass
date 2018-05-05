/* debug.c 
   created 15.4.96
   
*/
#include <stdio.h>
#include "my.h"

void mesg(char s[])
{ 
  printf("MESG: <%s>\n",s);
}

void mesgch(int c)
{ putch('<'); putch((char) c); putch('>'); }
