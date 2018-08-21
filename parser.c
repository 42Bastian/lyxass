#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "my.h"

#include "label.h"
#include "global_vars.h"
#include "error.h"


#define LOAD_BUFFER_SIZE (128*1024)  // hope this is enough

extern void ConvertFilename(char *);

int ch;
int next_ch;

int atom;
int next_atom;

char * srcLinePtr;
char srcLine[256];

char * loadBuffer;

/* prototypes */

int LoadSource(char fn[]);
int GetChar();
char * InsertMacroVar(char *ptr,int var);
int GetLine();

int GetAtom(void);
int KillSpace(void);
int GetCmd(void);
int GetComment(void);
int GetLabel(label_t *Label);

/********************************************/
/* get a source-file and add it to the list */
/********************************************/
char line[256];

int LoadSource(char fn[])
{
  FILE *f;

  ConvertFilename(fn);

  if ( (f = fopen(fn,"r")) != NULL ){
    char * ptr, *ptr_end;

    ptr = loadBuffer;

    ptr_end = ptr + LOAD_BUFFER_SIZE - 256;

    strcpy(file_list[Global.Files].name,fn);

    while ( ! feof(f)  && ptr <= ptr_end){
      char c;
      char *ptrLine = line;

      memset(line,0,254);
      fgets(line,254,f);
      ptrLine = line;
      // remove comment
      if ( ((c = *ptrLine++) == '*') ||
           (c == ';') ||
           (c == '/' && *ptrLine == '/') )
      {
        *ptr++ = '\n';
      } else {
        while ( c ){
          // convert a CR if found
          if ( c == '\r' ) c = ' ';

          // comment ? => finished
          if ( c == ';' ){
            while ( *(ptr-1) == ' ' ){
              --ptr;
              *ptr = 0;
            }
            *ptr++ = '\n';
            break;
          }

          // do not delete Space within strings
          if ( c == '"' ){
            do{
              *ptr++=c;
              c = *ptrLine++;
            }while ( c && c != '"' );
            *ptr++=c;
            c = *ptrLine++;
          }

          // else, copy anything
          while ( c && c != ' ' && c != '\t' &&
                  c != ';' && c != '"' && c != '/')
          {
            *ptr++ = c;
            c = *ptrLine++;
          }
          /* C++ comment */
          if ( c == '/' ){
            if ( *ptrLine == '/' ){
              *ptr++ = '\n';
              break;
            } else {
              *ptr++ = c;
              c = *ptrLine++;
            }
          }

          // remove Spaces...
          while ( c == ' ' || c == '\t'){
            c = *ptrLine++;
          }
          // ... leave only one
          if ( c && c != '\n' ) *ptr++ = ' ';
        }
      } // if ( c == '*' )
    } // while

    while( *(ptr-2) == ' ' ){
      --ptr;
      *ptr = '\n';
    }
    *ptr++ = 0;

    fclose(f);

    if ( ptr >= ptr_end ) Error(LOAD_ERR,fn);

    Current.SrcPtr = my_malloc( ptr-loadBuffer + 1);
    memcpy(Current.SrcPtr, loadBuffer, ptr-loadBuffer);

    Current.Line = 0;

  } else {
    Global.Files--;
    return Error(FILE_ERR,fn);
  }
  return TRUE;
}

/*-------------------------------------------------------------------------*/

int GetChar()
{
  if ( Current.SrcPtr ){
    ch = *Current.SrcPtr++;
    next_ch = *Current.SrcPtr;
  } else {
    ch = EOF;
    next_ch = EOF;
  }

  switch ( ch ){
  case 0 :
  case EOF:
    ch = EOF;
    Current.SrcPtr = NULL;
    break;
  case 0xA:
    ch = EOL;
    break;
  case '\r':             // replace CR by SPACE
  case '\t':
  case ' ':
    ch = SPACE;
    break;
  default:
    ;
  }
  return ( ch != EOF && ch != EOL );
}
/*
  If we are inside a macro, repalce \x by the text
  of the macro parameter.
*/
char * InsertMacroVar(char *ptr,int var)
{
  char *ptr2;

  ptr2 = macrovars[Current.Macro.Processing-1][var];
  while ( *ptr2 )
    *ptr++ = *ptr2++;

  return ptr;
}
/*
  Copy a line from the source-memory.
  Insert macor-parameters if needed.
*/
int GetLine()
{
  extern int mac_mode;
  char *ptr;

  int last_ch = 0;

  srcLinePtr = ptr = srcLine;
  memset((char *)ptr,0,256);

  memset((char *)&Current.Label, 0, sizeof(label_t));

  while ( GetChar() ){

    if ( Current.Macro.Processing ){
      if ( (last_ch != '.') && (ch == '\\') ){
        if ( last_ch == '.' ){
          *ptr++ = '\\';
        }
        if ( next_ch >= '0' && next_ch <= '9' ){
          int var;
          GetChar();
          var = ch - '0';
          if ( mac_mode ){
            if ( --var < 0 ){
              var = 9;
            }
          }
          if ( (next_ch >= '0' && next_ch <= '5')){
            GetChar();
            var = 10*var + ch-'0';
          }
          ptr = InsertMacroVar(ptr,var);
          continue;

          //
          // insert number of parameters
          //
        }else if ( next_ch == '#' ){
          char help[20];
          char *ptr1 = help;

          GetChar();
          sprintf(help,"%d",Current.Macro.NPara);
          while ( *ptr1 ){
            *ptr++ = *ptr1++;
          }
          continue;

          //
          // insert macro-name
          //
        } else if ( (next_ch == '?' && !mac_mode) || next_ch == '^' ){
          char *ptr1 = Current.Macro.Name;
          GetChar();
          while ( *ptr1 ){
            *ptr++ = *ptr1++;
          }
          continue;
        } else if ( next_ch == '~' ){
          char *ptr1 = (char *)calloc(10,1);

          GetChar();
          if ( mac_mode ){
            *ptr++='M';
          }
          sprintf(ptr1,"%04X",Current.Macro.invoked);

          while(*ptr1)
            *ptr++=*ptr1++;
          continue;
        } else if ( mac_mode && next_ch == '?' ){
          int var;
          GetChar();
          GetChar();
          var = ch - '1';
          if ( var < 0 ) {
            var = 9;
          }
          if ( macrovars[Current.Macro.Processing-1][var][0] ){
            *ptr++ = '1';
          } else {
            *ptr++ = '0';
          }
          continue;
        }

      }
    }
    last_ch = ch;
    *ptr++ = ch;

  }

  if ( srcLinePtr[255] ) exit(1);

  if ( ch == EOF && ptr == srcLinePtr ){
    return 0;
  } else {
    GetAtom();
    return 1;
  }
}

/*-------------------------------------------------------------------------*/
static char * save_srcLinePtr;

void SavePosition(void)
{
  save_srcLinePtr = srcLinePtr-1;
}

void RestorePosition(void)
{
  srcLinePtr = save_srcLinePtr;
  GetAtom();
}
//
// fetch next character from the current source-line
//
int GetAtom(void)
{
  atom = *srcLinePtr++;
  next_atom = *srcLinePtr;

  if ( !atom ) srcLinePtr--;

  return atom;
}

int TestAtom(int x)
{
  KillSpace();
  if ( atom == (x) ){
    GetAtom();
    return 1;
  }
  return 0;
}

int TestAtomOR(int x, int y)
{
  KillSpace();
  if ( atom == x || atom == y){
    GetAtom();
    return 1;
  }
  return 0;
}

int GetString(char * ptr, char endmark)
{
  while ( atom && atom != endmark ){
    *ptr++ = atom;
    GetAtom();
  }
  *ptr = 0;

  if ( TestAtom(endmark) ){
    return 1;
  } else {
    return 0;
  }
}

int KillSpace(void)
{
  while ( atom == SPACE )
    GetAtom();
  return ( atom );
}


void killLine(void)
{
  while ( atom )
    GetAtom();
}

int GetFileName()
{
  int save;
  char help[80];
  int i;

  KillSpace();
  if ( atom != '"' && atom != '<' && atom != '\'') return Error(SYNTAX_ERR,"");
  if ( atom == '<' )
    save = '>';
  else
    save = atom;

  GetAtom();

  if ( !GetString( help, save ) ) return  Error(SYNTAX_ERR,"");

  memset(filename,0,512);

  if ( save == '>' && bll_root && sourceMode == LYNX){
    strcpy(filename,bll_root);
  } else if ( save == '>' && bjl_root && sourceMode == JAGUAR){
    strcpy(filename,bjl_root);
  } else {
    strcpy(filename,Global.Path);
  }
  i = strlen(filename);
  if ( i && filename[i-1] != '/' && filename[i-1] != '\\'){
    filename[i] = '/';
  }
  if ( save == '\'' ) {
    char *ptr = help;
    char c;
    while ( (c = *ptr) ){
      *ptr++ = toupper(c);
    }
  }

  strcat(filename,help);

  return 0;
}
// GetLabel
// 4 types :
// 1. normal : alphanum + :
// 2. global : alphanum + ::
// 3. local  : . + alphanum
// 4. macro  : .\ + alphanum
//
int GetLabel(label_t *Label)
{
  extern int LabelTable[256];
  extern int mac_mode;

  int len = 0;
  char varhelp[6],*varptr = varhelp;
  char *ptr = Label->name;

  memset((char *)Label,0,sizeof(label_t));

  Label->type = UNSURE;

  sprintf(varhelp,"%04x",Global.var & 0xffff);

  if ( TestAtom('.') ){

    if ( !atom ) return  Error(LABEL_ERR,"");

    Label->type = LOCAL;

    if (atom != '\\' && LabelTable[atom]) {
      do {
        if ( atom == '@' ){
          varptr = varhelp;
          while( *varptr ){
            *ptr++ = *varptr++;
            len++;
          }
        } else {
          len++;
          *ptr++ = atom;
        }
      } while ( GetAtom() && LabelTable[atom] );

      if ( /*mac_mode &&*/ atom == ':' ){
        GetAtom();
      }
    } else {
      Label->type |= MACRO;

      if ( !GetAtom() ) return Error(LABEL_ERR,"");
      if ( atom == '~' ){
        char *ptr1 = (char *)calloc(10,1);

        GetAtom();
        if ( mac_mode ){
          *ptr++='M';
          ++len;
        }
        sprintf(ptr1,"%04X",Current.Macro.invoked);

        while(*ptr1){
          *ptr++=*ptr1++;
          ++len;
        }
      }
      if ( isalpha(atom) || atom == '_') {
        do{
          if ( atom == '@' ){
            varptr = varhelp;
            while( *varptr ){
              *ptr++ = *varptr++;
              len++;
            }
          } else {
            len++;
            *ptr++ = atom;
          }
        }while ( GetAtom() && LabelTable[atom] );
      }
      if ( /*mac_mode &&*/ atom == ':' ){
        GetAtom();
      }
    }
  } else if ( isalpha(atom) || atom == '_') {

    Label->type = UNSURE|NORMAL;
    do{
      if ( atom == '@' ){
        varptr = varhelp;
        while( *varptr ){
          *ptr++ = *varptr++;
          len++;
        }
      } else if ( atom == '$' ){
        label_t label;
        char help[10], *p = help;
        int32_t l;

        GetAtom();
        if ( GetLabel( &label ) ) return 1;
        if ( !FindLabel( &label, &l) ) return 1;

        sprintf(help,"%08X",l);
        while ( *p ){
          *ptr++=*p++;
        }
        break;
      } else {
        len++;
        *ptr++ = atom;
      }
    }while ( GetAtom() && (LabelTable[atom] || atom == '$'));
  } else if ( atom == '\\' && Current.Macro.Define ){
    GetAtom();
    if ( (atom >= '0' && atom <= '9') ||
         (atom >= 'a' && atom <= 'f') ||
         (atom >= 'A' && atom <= 'F')){
      GetAtom();
      return 0;
    }
  }
  *ptr++ = 0;
  // must return here 1 for GPU/DSP mode
  if ( len == 0 ) return 1; //Error(LABEL_ERR,"");

  if ( atom == ':' ){
    GetAtom();
    if ( !(Label->type & NORMAL) && !mac_mode ) return Error(LABEL_ERR,"");

    Label->type  = NORMAL;

    if ( atom == ':'  ){
      Label->type = GLOBAL;
      GetAtom();
    }
  }

  Label->len   = len;
  Label->file  = Current.File;
  Label->line  = Current.Line;
  Label->value = Global.pc;

  //  printf("GetLabel:%d(%s)%p %p",len,Label->name,Label->name,ptr);

  return 0;
}

int GetCmd(void)
{
  extern int LabelTable[256];

  int len = 0;
  char *ptr = Current.Cmd;

  memset(Current.Cmd, 0, 80);
  Current.CmdLen = 0;

  if ( KillSpace() ){
    if (isalpha(atom) || atom == '_' || atom == '.' )
      do{
        *ptr++ = atom;
        len++;
      }while ( GetAtom() && (LabelTable[atom] || atom == '.') );
  }
  Current.CmdLen = len;
  *ptr++ = 0;
  return (len == 0);
}

int GetComment(void)
{
  if ( !KillSpace() ) return 0;

  if ( atom != ';' ){
    return 1;
  }

  return 0;
}

void InitParser(void)
{
  loadBuffer = my_malloc(LOAD_BUFFER_SIZE);
}
