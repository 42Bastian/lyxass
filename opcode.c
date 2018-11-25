/*

  opcode.c

  search opcode and execute function

*/


#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "global_vars.h"
#include "opcode.h"

extern int sourceMode;

/* SearchOpcode

   looks through opcode-list and jumps into opcode-function

   returns either error-code or -1 for no opcode found
*/
int SearchOpcode(const struct opcode_s *list, const char *s)
{
  const struct opcode_s *curr;

  if ( strlen(s) > 7 ){
    return -1;
  }
  curr = list;
  while ( curr->name[0] ){
    if ( !strcmpi(curr->name,s) ){
      //      printf("Found (%s)\n",curr->name);
      return curr->func( curr->misc );
    }
    curr++;
  }
  return -1;
}

int SearchOpcode2(const struct opcode_s *list,
                  const char *s,
                  int (**fun)(int ),
                  int *para)
{
  const struct opcode_s *curr;
  extern int mac_mode;

  // ignore leading dot
  if ( s[0] == '.' ){
    ++s;
  }
  if ( strlen(s) > 7 && !mac_mode ){
    return -1;
  }

  if ( Global.mainMode != JAGUAR && !strcmpi(s,"DP") ){
    return -1;
  }
  curr = list;

  while ( curr->name[0] ){
    if ( !strcmpi(curr->name,s) ){
      //      printf("Found (%s)\n",curr->name);
      *fun = curr->func;
      *para = curr->misc;
      return 1;
    }
    curr++;
  }
  return -1;
}
