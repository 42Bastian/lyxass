
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my.h"
#include "error.h"
#include "label.h"
#include "global_vars.h"

extern char srcLine[256];  // parser.c

char lineBuffer[256];

REFERENCE *refFirst = 0;
REFERENCE *refLast = 0;

/***********************/
/* save current line   */
/***********************/
void saveCurrentLine()
{
  REFERENCE *refPtr;
  
  refPtr = (REFERENCE *)my_malloc(sizeof(REFERENCE) + strlen(srcLine)+1 );

  strcpy(refPtr->src,srcLine);
  refPtr->line = Current.Line-1;
  refPtr->file = Current.File;
  refPtr->macroline = Current.Macro.Line;
  refPtr->macrofile = Current.Macro.File;
  refPtr->macroinvoked = Current.Macro.invoked;
  refPtr->macro = Current.Macro.Processing;
  //  refPtr->var = Current.var;
  refPtr->pc = Global.OldPC;
  refPtr->codePtr = code.OldPtr;
  refPtr->up =
  refPtr->down = (REFERENCE *)0;
  refPtr->unknown = unknownLabel;
  refPtr->mode = sourceMode;

  if ( refFirst ){
    refPtr->down = refLast;
    refLast->up = refPtr;
  } else {
    refFirst = refPtr;
  }
  refLast = refPtr;

//->   printf("ref.c: %s (%s) = %p\n",unknownLabel->name,srcLine,refLast);
}
