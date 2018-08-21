/*
  label.c

  Handle labels.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "my.h"
#include "label.h"
#include "error.h"
#include "global_vars.h"


void DumpLocals();
void CheckForUnsolvedLocals();

label_t * hash[256];

// base for various labels
label_t *local_labels;
label_t *global_labels;
label_t *macro_labels;
label_t *macronames;

// ptrs to the next free space
label_t *next_local;
label_t *next_global;
label_t *next_macro;
label_t *next_macroname;

// ptrs to chained labels
label_t *local_hook = NULL;
label_t *macro_hook = NULL;
label_t *macroname_hook = NULL;

int n_local = 0;
int n_global = 0;
int n_localmacro = 0;
int n_macro = 0;

int LabelTable[256];

void InitLabels(void)
{
  int c,c1;

  memset((char *) hash, 0, 256*sizeof(label_t *));
  next_local =
    local_labels = (label_t *)my_malloc( sizeof(label_t) * MAX_LOCAL_LABELS );
  next_global =
    global_labels = (label_t *)my_malloc( sizeof(label_t) * MAX_GLOBAL_LABELS );
  next_macro =
    macro_labels = (label_t *)my_malloc( sizeof(label_t) * MAX_MACRO_LABELS );
  next_macroname =
    macronames = (label_t *)my_malloc( sizeof(label_t) * MAX_MACROS);

  memset((char *)LabelTable, FALSE, 256 * sizeof(int) );

  for (c = 'A',c1 = 'a'; c <= 'Z'; c++,c1++){
    LabelTable[ c ] = TRUE;
    LabelTable[ c1 ]= TRUE;
  }
  for (c = '0'; c <= '9'; c++)
    LabelTable[ c ] = TRUE;

  LabelTable[(int)'_'] = TRUE;
  LabelTable[(int)'@'] = TRUE;
  LabelTable[(int)'.'] = TRUE;
}

int ComputeHash(char * s)
{
  int hashval = 0;
  int pos = 0;
  while ( *s ){
    hashval ^= pos;
    hashval += (int) *s;
    ++s;
    ++pos;
  }
  return (hashval & 255);
}
void ClearLocals()
{
  CheckForUnsolvedLocals();
  next_local = local_labels;
  local_hook = NULL;
  memset((char *)local_labels, 0,  sizeof(label_t)*MAX_LOCAL_LABELS);
}

label_t * DefineLabel(label_t *l, int *solved)
{
  int hashvalue;
  label_t *ptr;
  label_t *_this;
//->  extern int mac_mode;

  *solved = 0;

  //mesg("DefineLabel");

  //  printf("Type:%02x\n",l->type);

  if ( (l->type & GLOBAL) || (l->type & NORMAL) ){

    if ( n_global == MAX_GLOBAL_LABELS ){
      Error(TOOMANY_ERR,"");
      return NULL;
    }

    // clear local-labels
    if ( (l->type & GLOBAL) /*|| mac_mode*/ ){

      CheckForUnsolvedLocals();
      next_local = local_labels;
      local_hook = NULL;
      memset((char *)local_labels, 0, sizeof(label_t)*MAX_LOCAL_LABELS);

    }
    // get hash-value

    hashvalue = ComputeHash(l->name);

    //printf("Hashvalue: %d %p ",hashvalue,hash[hashvalue]);

    // check first hash-tab.
    if ( (ptr = hash[hashvalue]) == NULL ){
      hash[hashvalue] = next_global;
    }

    // go through all lables, check if _this one is already used
    // if not, go until the end is reached.

    else {
      do{
	if ( ptr->len == l->len &&
	     ! strcmp(l->name, ptr->name) ){

	  if ( (ptr->type & REGISTER) ){
	    l->type |= REGISTER;
	    l->file = Current.File;
	    return ptr;
	  } else if ( (ptr->type & VARIABLE) ){
	    //printf("LABEL:(%s)%d\n",next->name,next->value);
	    l->type |= VARIABLE;
	    return ptr;
	  } else if ( ptr->type & UNSOLVED ){
	    l->next = ptr->next;
	    *ptr = *l;
	    *solved = 1;
	    return ptr;
	  } else {
	    Error(REDEF_ERR,(char *) ptr);
	    return NULL;
	  }
	}
	if ( ! ptr->next ) break;
	ptr = ptr->next;
      } while ( 1 );
      ptr->next = next_global;
    }

    // copy temp. data into buffer
    _this = next_global;

    *next_global++ = *l;
    ++n_global;

    //    printf("next_global:%p\n",next_global);
    return _this;
  } // global or normal label

  /* define a macro-label : .\xxx */

  if ( (l->type & (MACRO|LOCAL)) == (MACRO|LOCAL) ){

    //mesg("macro-label");

    if ( n_localmacro == MAX_MACRO_LABELS ){
      Error(LMTOOMANY_ERR,"");
      return NULL;
    }

    if ( (ptr = macro_hook) == NULL){
      macro_hook = next_macro;
    } else {
      do{
	if ( (ptr->len == l->len) &&
	     ! strcmp(l->name, ptr->name) ){
	  if ( ptr->type & VARIABLE ){
	    //	    printf("LABEL:(%s)%d\n",ptr->name,ptr->value);
	    l->type |= VARIABLE;
	    return ptr;
	  } else if ( ptr->type & UNSOLVED ){
	    l->next = ptr->next;
	    *ptr = *l;
	    *solved = 1;
	    return ptr;
	  } else {
	    // re-definition allowed
	    ptr->line = l->line;
	    ptr->file = l->file;
	    ptr->value = l->value;
	    return ptr;
	    //	    Error(REDEF_ERR,(char *) ptr);
	    //	    return NULL;
	  }

	}
	if ( !ptr->next ) break;
	ptr = ptr->next;
      } while ( 1 );
      ptr->next = next_macro;
    }

    l->next = (label_t *)NULL;
    _this = next_macro;
    *next_macro++ = *l;
    ++n_localmacro;
    //    printf("macro %s %02x",_this->name,_this->type);
    return _this;
  }

  /* define a local label : .xxx */

  if ( (l->type & LOCAL) == LOCAL  ){

    //mesg("local label");

    if ( n_local == MAX_LOCAL_LABELS ){
      Error(LTOOMANY_ERR,"");
      return NULL;
    }

    if ( (ptr = local_hook) == NULL){
      local_hook = next_local;
    } else {
      do{
	if ( (ptr->len == l->len) &&
	     ! strcmp(l->name, ptr->name) ){
	  if ( ptr->type & VARIABLE ){
	    //printf("LABEL:(%s)%d\n",ptr->name,ptr->value);
	    l->type |= VARIABLE;
	    return ptr;
	  } else if ( ptr->type & UNSOLVED ){
	    l->next = ptr->next;
	    *ptr = *l;
	    *solved = 1;
	    return ptr;
	  } else {
	    Error(REDEF_ERR,(char *) ptr);
	    return NULL;
	  }
	}
	if ( !ptr->next ) break;
	ptr = ptr->next;
      } while ( 1 );

      ptr->next = next_local;
    }

    l->next = (label_t *)NULL;
    _this = next_local;
    *next_local++ = *l;
    ++n_local;

    //    printf("%s (%d,%d)-%s-\n ",l->name,l->line,l->value,srcLine);
    return _this;
  }

  /* define a macro-name */

  if ( l->type == MACRO  ){

    // mesg("macro - name");

    if ( n_macro == MAX_MACROS ){
      Error(MTOOMANY_ERR,"");
      return NULL;
    }

    if ( (ptr = macroname_hook) == NULL){
      macroname_hook = next_macroname;
    } else {

      for ( ; ptr->next ; ptr = ptr->next ){
	if ( (ptr->len == l->len) &&
	     ! strcmp(l->name, ptr->name) ){
	  Error(REDEF_ERR,(char *) ptr);
	  return NULL;
	}
      }
      ptr->next = next_macroname;
    }

    l->next = (label_t *)NULL;
    _this = next_macroname;
    *next_macroname++ = *l;
    ++n_macro;

    return _this;
  }
  return NULL;
}
//---------------------------------------------------------------------
// FindLabel
//---------------------------------------------------------------------
label_t * FindLabel(label_t *l, int32_t *value)
{
  label_t *next;

  if ( l->type & LOCAL ){
    next = local_hook;
    if ( l->type & MACRO ){
      next = macro_hook;
    }
  } else {
    next = hash[ComputeHash(l->name)];
  }

  for ( ; next; next = next->next ){

    //  printf("FindLabel:(%s)%p\n",next->name,next->next);

    if ( (l->len == next->len) && !strcmp(l->name,next->name)){
      *value = next->value;
      l->type = next->type;
      if ( next->type & UNSOLVED )
	return 0;
      else
	return next;
    }
  }
  return 0;
}

int FindMacro(char *s, label_t ** macro)
{
  label_t *ptr = macroname_hook;

  while ( ptr ){
    if ( ! strcmp(s,ptr->name) ){
      break;
    }
    ptr = ptr->next;
  }
  if ( ! ptr ){
    return 1;
  }

  *macro = ptr;

  return 0;
}

void undefAllMacroLocals()
{
  label_t * ptr = macro_hook;

  for ( ; ptr ; ptr = ptr->next){
    ptr->type |= UNSOLVED;
  }
}
/**********************************************/
/* dump global labels                         */
/**********************************************/
extern FILE * my_stderr;

void DumpGlobals()
{
  label_t * next;
  int i;
  fprintf(my_stderr,"Global labels:\n");
  for (i = 0; i < 256; ++i){
    for( next = hash[i] ; next ; next = next->next ){
      if ( !(next->type & UNSOLVED) && (next->type & GLOBAL)){
	fprintf(my_stderr,"<%32s> = %08x %02x [%5d %s]\n",
		next->name,next->value,next->type,
                next->line,file_list[next->file].name);
      }
    }
  }
}

void writeSymbols(char *fn, int hex)
{
  label_t * next;
  int i;
  FILE *fh;
  char *help = fn + strlen(fn);

  while ( *help != '.' )
    --help;

  ++help;
  *help = 0;

  help = my_malloc(strlen(fn)+4);
  strcpy(help,fn);
  strcat(help,"equ");
  if ( (fh = fopen(help,"w")) == NULL ){
    Error(WRITE_ERR,fn);
    return;
  }

  for (i = 0; i < 256; ++i){
    for( next = hash[i] ; next ; next = next->next ){
      if ( !(next->type & UNSOLVED) && (next->type & GLOBAL)){
        if ( hex ){
          fprintf(fh,"%-24s EQU $%08x\n",next->name,next->value);
        } else {
          fprintf(fh,"%-24s EQU %d\n",next->name,next->value);
        }
      }
    }
  }
  fclose(fh);
}

void hashStatistics()
{
  int i,total = 0;
  label_t *next;
  for(i = 0 ; i < 256;++i){
    int o = 0;
    for (next = hash[i]; next; next = next->next ){
      ++o;
    }
    total += o;
    printf("%4d ",o);
  }
  printf("\n total:%d\n",total);
}

void DumpLocals()
{
  label_t * next = local_labels;
  fprintf(my_stderr,"Local labels:\n");
  for ( ; next ; next = next->next ){
    fprintf(my_stderr,"<%32s> = %04x [%5d %s]\n",
            next->name,next->value,next->line,file_list[next->file].name);
  }
}

void DumpMacros()
{
  label_t * next = macroname_hook;
  fprintf(my_stderr,"Macros:\n");
  for ( ; next ; next = next->next ){
    fprintf(my_stderr,"<%32s> [%5d %s]\n",
            next->name,next->line,file_list[next->file].name);
  }
}
