/*

  opcode.c

  search opcode and execute function

*/


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "my.h"
#include "error.h"
#include "label.h"
#include "global_vars.h"

#include "opcode.h"


/* SearchOpcode

   looks through opcode-list and jumps into opcode-function

   returns either error-code or -1 for no opcode found
*/
#include <string.h>
#include <ctype.h>

int SearchOpcode(const struct opcode_s *,char *);
int SearchOpcode(const struct opcode_s *list,char *s)
{
  const struct opcode_s *curr;
  char search[12];
  int i;
  i = strlen(s);

  if ( i > 7 ){
    return -1;
  }

  search[i] = 0;

  for(i = strlen(s) -1; i >= 0 ; --i){
    search[i] = toupper(s[i]);
  }

  // get opcode and transfer into upper-case

  curr = list;

//->  mesg(search);
  while ( curr->name[0] ){

    //mesg(curr->name);

    if ( !strcmp(curr->name,search) ){
      //      printf("Found (%s)\n",curr->name);
      return curr->func( curr->misc );
    }
    curr++;
  }
  return -1;
}

int SearchOpcode2(const struct opcode_s *,char *, int (**)(int ),int *);
int SearchOpcode2(const struct opcode_s *list,char *s,int (**fun)(int ), int *para)
{
  const struct opcode_s *curr;
  uint32_t search1,search2;
  extern int mac_mode;

  // ignore leading dot
  if ( /*mac_mode && */ s[0] == '.' ){
    memmove(s,s+1,strlen(s));
  }
  if ( strlen(s) > 7 ){
    if ( !mac_mode ){
      return -1;
    }
  }

  // get opcode and transfer into upper-case

  search1 = (*(uint32_t *)s) & 0xdfdfdfdf;
  search2 = (*(uint32_t *)(s+4)) & 0xdfdfdfdf;

  curr = list;

  while ( curr->name[0] ){

    //mesg(curr->name);

    if ( search1 == *(uint32_t *)curr->name && search2 == *(uint32_t *)(curr->name+4) ){
      //      printf("Found (%s)\n",curr->name);
      *fun = curr->func;
      *para = curr->misc;
      return 1;
    }
    curr++;
  }
  return -1;
}
