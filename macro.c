/*

  macro.c

  handles macros

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "my.h"
#include "error.h"
#include "label.h"
#include "parser.h"
#include "global_vars.h"

int GetAtom(void);
extern int mainloop(int);
extern int cntMacroExpand;

char macrovars[16][16][80];

/*
  DefineMacro

  save all lines until ENDM

*/

static int recur = 0;
LABEL macroName;

int DefineMacro(char *s)
{

  if ( recur ){
    ++recur;
    if ( recur > 2 ){
      Error(MACRONESTED_ERR,"");
      return 1;
    }
    return 0;
  }

  ++recur;

  macroName.len = strlen(s);
  strcpy(macroName.name,s);
  macroName.line = Current.Line;
  macroName.file = Current.File;
  macroName.value = (long) Current.SrcPtr;
  macroName.type = MACRO;
  Current.Macro.Define=1;

  return 0;
}

extern int error; 

int EndDefineMacro(){
  
  int dummy;

  if ( --recur ){
    return 0;
  }
  Current.Macro.Define = 0;

  if ( ! DefineLabel(&macroName, &dummy) ){
    return 1;
  }
  if ( dummy ) return Error(SYNTAX_ERR,"");

  return 0;
}  

int GetOperand(char * dst)
{
  int StringFlag = 0;
  int ParenFlag = 0;
  int exit = FALSE;
  char *ptr = dst;
  
  if ( KillSpace() ){
    while ( atom  && !exit ) {
      switch ( atom ){
      case  '"' :
	StringFlag ^= 1;
	*ptr++=atom;
	break;
      case '{' : 
	ParenFlag ++;
	break;
      case '}':
	ParenFlag --;
	break;
      case ',':
	if  ( !StringFlag && !ParenFlag ){
	  exit = TRUE;
	  continue;
	} else
	  *ptr++ = atom;
	break;
      case ';':
	if ( !StringFlag && !ParenFlag ){
	  exit = TRUE;
	  continue;
	} else
	  *ptr++ = atom;
	break;
      case SPACE:
	if ( StringFlag || ParenFlag ){
	  *ptr++ = atom;
	}
	break;
      default:
	*ptr++ = atom;
      }
      GetAtom();
    }
  }
  *ptr=0;
  return ptr != dst;
}  

extern void undefAllMacroLocals();

int CheckMacro(char * s)
{
  int err = 0;  

  if ( Current.ifFlag && Current.switchFlag ){
    int OpCnt = 0;
    int save_atom = atom;
    struct current_s save_Current = Current;
    LABEL *macro;

    if ( FindMacro( s , &macro) ) return -1;

    //printf("Macro: (%s)\n",macro->name);

    Current.Macro.Line = Current.Line;
    Current.Macro.File = Current.File;
    Current.Line = macro->line;
    Current.File = macro->file;
    Current.Macro.Name = macro->name;
    Current.Macro.invoked = ++macro->count;
    Current.SrcPtr = ((char *)macro->value);

    memset((char *)macrovars[Current.Macro.Processing],0,16*80);

    do {
      if ( GetOperand(macrovars[Current.Macro.Processing][OpCnt]) ||
	   atom == ',' ) OpCnt++;
    }while ( TestAtom(',') && OpCnt <= 16);

    //    printf("%d macro-paras\n",OpCnt);

    Current.Macro.NPara = OpCnt;
    Current.Macro.Processing++;
    
    undefAllMacroLocals();

    ++cntMacroExpand;
    err = mainloop(0);

    Current = save_Current;
    atom = save_atom;
  }
  return err;
}
