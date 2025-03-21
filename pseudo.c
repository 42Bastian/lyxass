/*
  pseudo.c

  check/execute pseudo-opcodes

*/

#include <stdio.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "my.h"
#include "error.h"
#include "label.h"
#include "global_vars.h"
#include "parser.h"
#include "output.h"
#include "pseudo.h"

// mnemonic.c
int Endian(void);

// lyxass.c
int mainloop(int );
int LoadFile(char *, long , long , char *, long *);

// ref.c
void saveCurrentLine();

// macro.c
int DefineMacro(char *);
int EndDefineMacro();

extern FILE *my_stderr;
extern int verbose;

// keep track if a register is used
typedef struct regLabel {
  int reg_flag[32*2];
  int reg_line[32*2];
  int reg_file[32*2];
  label_t *reg_label[32*2];
  int regAllocStart;
} regLabel_t;

regLabel_t risc[2] = {
  {.regAllocStart = 29 },
  {.regAllocStart = 29 }
};

static char transASCII[256];

int mac_mode = 0;

void translate(char * s)
{
  while ( *s ){
    *s = transASCII[(int)*s];
    s++;
  }
}

void InitTransASCII(void)
{
  int i;
  for ( i = 255; i >= 0; --i){
    transASCII[i] = (char)i;
  }
}
/*

  ORG

*/
int p_org(int d)
{
  int32_t l;
  int err;

  if ( (err = NeedConst( &l, "ORG" )) ) return err;

  if ( Global.genesis == 1 && mac_mode == 0){
    if ( (uint32_t)l < Global.pc ) return Error(SYNTAX_ERR,"");
    writeSameBytes( 0, l-Global.pc );
  } else {
    Global.pc = (uint32_t)l;
  }

  return 0;
}
/*

  ORG

*/
int p_porg(int d)
{
  int32_t l;
  int err;

  if ( (err = NeedConst( &l, "ORG" )) ) return err;

  Global.pc = (uint32_t)l;

  return 0;
}


/*

  RUN

*/
int p_run(int d)
{
  int err;
  int32_t l;

  KillSpace();
  if ( !atom ) {
    l = Global.pc;
  } else {
    if ( (err = NeedConst( &l, "RUN" )) ) return err;
  }
  if ( Global.run == -1){
    Global.mainMode = sourceMode;
    Global.run = l;
    Global.genesis = 1;
  }
  Global.pc = l;
  return 0;
}
/*

  END

*/
int p_end(int d)
{
  *Current.SrcPtr=0;
  atom = EOF;
  return 0;
}
/*

  SET@

*/
int p_setvar(int d)
{
  int32_t l;
  int err;

  if ( (err = NeedConst( &l, "SET@" )) ) return err;

  Global.var = l;

  return 0;
}
/*

  INC@

*/
int p_incvar(int d)
{
  Global.var += d;

  return 0;
}
/*

  MACRO

*/


int p_macro(int d)
{
  label_t macroLabel;

  //  KillSpace();

  if ( GetLabel( &macroLabel ) ) return 1;
  if ( ! macroLabel.len)  return 1;

  return DefineMacro( macroLabel.name );
}
/*

  ENDM

*/
int p_endm(int d)
{
  if ( Current.Macro.Define ){
    return EndDefineMacro();
  } else {
    Current.SrcPtr = 0;
    return 0;
  }
}
/*

  DC.B, DB

*/
int p_definebyte(int d)
{
  char help[80];
  int err;
  int32_t l;
  int all_err = 0;

  KillSpace();
  if ( atom == EOF ) return Error(SYNTAX_ERR, "Missing String");
  do{
    if ( TestAtom('"') ){
      if ( !GetString( help ,'"' ) ) return Error(SYNTAX_ERR, "Missing '\"'");
      if ( d ){
        translate(help);
      }
      writeBytes(help,(int)strlen(help));
    }
    else if ( TestAtom('\'') ){
      if ( !GetString( help ,'\'' ) ) return Error(SYNTAX_ERR, "Missing \"'\"");
      if ( d ){
        translate(help);
      }
      writeBytes(help,(int)strlen(help));
    }
    else {
      if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
      if ( err == EXPR_UNSOLVED ) ++all_err;
      if ( l < -128 || l > 255 ) return Error(BYTE_ERR,"");
      if ( d ) l = transASCII[l];
      writeByte((char)l);
    }
  } while ( TestAtom(',') );

  if ( all_err ) saveCurrentLine();
  return 0;
}
/*

  DC.W,DW

*/
int p_defineword(int d)
{
  int err;
  int32_t l;
  int all_err = 0;

  KillSpace();
  if ( atom == EOF ) return Error(SYNTAX_ERR,"");
  do{
    if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
    if ( err == EXPR_UNSOLVED ) ++all_err;
    if ( l < -32768L || l > 65535L ) return Error(WORD_ERR,"");
    if ( Endian() == targetLITTLE_ENDIAN ){
      writeWordLittle((short)l);
    }else{
      writeWordBig((short)l);
    }
  } while ( TestAtom(',') );

  if ( all_err ) saveCurrentLine();
  return 0;
}
/*

  DC.L,DL

*/
int p_definelong(int d)
{
  int err;
  int32_t l;
  int all_err = 0;

  KillSpace();
  if ( atom == EOF ) return Error(SYNTAX_ERR,"");
  do{
    if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
    if ( err == EXPR_UNSOLVED ) ++all_err;
    if ( Endian() == targetLITTLE_ENDIAN ){
      writeLongLittle( (long)l );
    }else{
      writeLongBig( (long)l );
    }
  } while ( TestAtom(',') );

  if ( all_err ) saveCurrentLine();
  return 0;
}
/*

  DC.P,DP

*/
int p_definephrase(int d)
{
  int err;
  int64_t ll;
  int all_err = 0;

  KillSpace();
  if ( atom == EOF ) return Error(SYNTAX_ERR,"");
  do{
    if ( (err = Expression64( &ll )) == EXPR_ERROR ) return 1;
    if ( err == EXPR_UNSOLVED ) ++all_err;
    if ( Endian() == targetLITTLE_ENDIAN ){
      writePhraseLittle( ll );
    }else{
      writePhraseBig( ll );
    }
  } while ( TestAtom(',') );

  if ( all_err ) saveCurrentLine();
  return 0;
}
/*

  TRANS

*/
int p_trans(int d)
{
  long len;
  int32_t offset = 0;

  if ( GetFileName() ) return 1;

  if ( TestAtom(',') ){
    if ( NeedConst( &offset ,"TRANS" ) ) return 1;
  }

  if ( LoadFile(transASCII, offset, 256, filename, &len ) ) return 1;

  return 0;
}
/*

  INCLUDE

*/
int p_include(int d)
{
  int err;

  if (Global.Files == MAX_INCLUDE) return Error(-INCLUDE_ERR,"");

  if ( GetFileName() ) return 1;

  KillSpace();

  if ( atom ) return Error(GARBAGE_ERR,srcLine);

  ClearLocals();

  struct current_s save_current = Current;

  Global.Files++;
  LoadSource(filename);
  Current.File = Global.Files;
  err = mainloop(0);

  Current = save_current;

  return err;
}
/*

  IBYTES

*/
int p_ibytes(int d)
{
  int32_t offset = 0;

  if ( GetFileName() ) return 1;

  if ( TestAtom(',') ){
    if ( NeedConst( &offset ,"IBYTES" ) ) return 1;
  }

  if ( Global.genesis ){
    long len;

    if ( LoadFile(code.Ptr, (long)offset, MAX_CODE_SIZE-code.Size, filename, &len ) ) return 1;

    code.Ptr += len;
    Global.pc += len;
    code.Size += len;
  }
  return 0;
}
/*

  PATH

*/
int p_path(int d)
{
  int i;

  KillSpace();

  memset(filename, 0, 512);

  if ( !atom ){
    Global.Path[0] = 0;
    return 0;
  }

  if ( atom != '"' ) return Error(SYNTAX_ERR,"");
  GetAtom();

  if ( !GetString( filename, '"' ) ) return Error(SYNTAX_ERR,"Missing '\"'");

  i = (int)strlen(filename);
  if ( i && filename[i-1] != '/'){
    filename[i] = '/';
  }

  strcat(Global.Path,filename);
  if ( chgPathToUpper ){
    char *ptr = Global.Path;
    char c;
    while ( (c = *ptr) )
      *ptr++ = toupper(c);
  }

  return 0;

}
/*

  DS.B,DS.W,DS.L

*/
int p_definespace(int size)
{
  int32_t l;

  if ( NeedConst( &l ,"DS.x") ) return 1;

  writeSameBytes( 0, l * size);
  return 0;
}
/*

  EQU

*/
int p_equ(int d)
{
  int32_t l;

  if ( NeedConst( &l, "EQU") ) return 1;

  if ( ! Current.Label.len ){
    Error(SYNTAX_ERR,"");
  }
  if ( (Current.Label.type &  REGISTER) ){
    Error(SYNTAX_ERR,"Cannot EQU a register label");
  }

  if ( (Current.Label.type & VARIABLE) == 0){
    Current.LabelPtr->value = l;
  }
  Current.Label.type &= ~CODELABEL;
  return 0;
}
/*

  SET

*/
int p_set(int d)
{
  int32_t l;

  if ( NeedConst( &l, "SET") ) return 1;

  if ( ! Current.Label.len ) Error(SYNTAX_ERR,"");

  if ( (Current.Label.type &  REGISTER) ){
    Error(SYNTAX_ERR,"Cannot EQU a register label");
  }


  Current.LabelPtr->type |= VARIABLE;
  Current.LabelPtr->type &= ~CODELABEL;
  Current.LabelPtr->value = l;
  Current.varModifier++;
  //  printf("(%s)%d\n",Current.LabelPtr->name,Current.LabelPtr->value);
  return 0;
}
/*

  IF

*/
#define PUSH_IF() Current.ifSave[++Current.ifCnt] = Current.ifFlag; Current.parseSave[Current.ifCnt] = Current.ParseOnly
#define POP_IF()  Current.ParseOnly = Current.parseSave[Current.ifCnt];Current.ifFlag = Current.ifSave[Current.ifCnt--]

int p_if(int d)
{
  int32_t l;

  if ( Current.ifCnt+1 == MAX_IF ) return Error(TOOMANYIF_ERR,"");

  PUSH_IF();

  if ( Current.ifFlag ) {
    TestAtom('#');             // skip a '#'

    if ( NeedConst( &l, "IF") ) return 1;

    Current.ifFlag = l != 0 ? 1 : 0;
    Current.ParseOnly = 0;
  } else {
    Current.ParseOnly = 1;
  }

  return 0;
}
/*

  IFDEF

*/
int p_ifdef(int d)
{
  int32_t l;
  label_t test;

  if ( Current.ifCnt+1 == MAX_IF ) return Error(TOOMANYIF_ERR,"");

  PUSH_IF();

  if ( Current.ifFlag ) {

    if ( GetLabel( &test ) ) return 1;

    Current.ifFlag = (FindLabel( &test, &l ) != 0);

    Current.ParseOnly = 0;
  } else {
    Current.ParseOnly = 1;
  }
  return 0;
}
/*

  IFUNDEF

*/
int p_ifundef(int d)
{
  int32_t l;
  label_t test;

  if ( Current.ifCnt+1 == MAX_IF ) return Error(TOOMANYIF_ERR,"");

  PUSH_IF();

  if ( Current.ifFlag ) {
    if ( GetLabel( &test ) ) return 1;
    Current.ifFlag = (FindLabel( &test, &l ) == 0);
    Current.ParseOnly = 0;
  } else {
    Current.ParseOnly = 1;
  }
  return 0;
}
/*

  IFVAR

*/
int p_ifvar(int d)
{

  if ( Current.ifCnt+1 == MAX_IF ) return Error(TOOMANYIF_ERR,"");

  PUSH_IF();

  if ( Current.ifFlag ) {
    KillSpace();

    Current.ifFlag = (atom != 0) ? 1 : 0;

    Current.ParseOnly = 0;
  } else {
    Current.ParseOnly = 1;
  }
  return 0;
}
/*

  ELSE

*/
int p_else(int d)
{
  if ( !Current.ParseOnly ){
    Current.ifFlag ^= 1;
  }
  return 0;
}
/*

  ENDIF

*/
int p_endif(int d)
{
  if ( ! Current.ifCnt ) Error(ENDIF1_ERR,"");

  POP_IF();

  return 0;
}
/*

  SWITCH

*/
int p_switch(int d)
{
  int32_t l;

  if ( Current.switchCnt ) return Error(SWITCH_ERR,"");

  if ( NeedConst( &l, "SWITCH") ) return 1;

  Current.switchCnt++;
  Current.switchValue = l;
  Current.switchFlag = 0;
  Current.switchCaseMatch = 0;
  return 0;
}
/*

  CASE

*/
int p_case(int d)
{
  int32_t l;

  if ( !Current.switchCnt ) return Error(CASE_ERR,"");

  if ( NeedConst( &l , "CASE") ) return 1;

  Current.switchFlag = (l == Current.switchValue);

  Current.switchCaseMatch |= Current.switchFlag;

  // printf("case: %d %d\n",Current.switchFlag,Current.switchCaseMatch);

  return 0;
}
/*

  ELSES

*/
int p_default(int d)
{
  if ( !Current.switchCnt) return Error(DEFAULT_ERR,"");

  Current.switchFlag = !Current.switchCaseMatch;

  //  printf("elses: %d %d\n",Current.switchFlag,Current.switchCaseMatch);

  return 0;
}
/*

  ENDS

*/
int p_ends(int d)
{
  if ( !Current.switchCnt ) return Error(ENDS_ERR,"");

  Current.switchFlag = 1;
  Current.switchCnt = 0;

  return 0;
}
/*

  ALIGN

*/
int p_align(int d)
{
  int32_t l1,l;
  if ( !(l = d) ){
    l = 2;
    KillSpace();
    if ( atom ){
      if ( NeedConst( &l,"ALIGN") ) return 1;
    }
  }
  l1 = l - ((int32_t)Global.pc) % l;
  if ( l != l1 ) writeSameBytes( 0, l1);

  return 0;
}
/*

  ECHO / FAIL

*/
int p_echo(int d)
{
  label_t label;
  int32_t l;

  if ( !TestAtom('"') ) return Error(SYNTAX_ERR, __FUNCTION__);

  if ( d ) {
    if (Current.File >= 0){
      if ( Current.Macro.Line ){
        fprintf(my_stderr,"FAIL: %s:%d:Macro-Line %5d:<%s>:",
                file_list[Current.File].name,Current.Line,
                Current.Macro.Line,file_list[Current.Macro.File].name);
      }
    } else {
      fprintf(my_stderr,"FAIL: %s:%5d :",
              Current.File >= 0 ? file_list[Current.File].name:"",Current.Line);
    }
  }

  while ( atom != '"' ){
    if ( atom == '%' && next_atom != '%'){
      int mode = 0;

      GetAtom();
      if ( atom == 'H' || atom == 'h' || atom == 'X' || atom == 'x'){
        mode = 1;
        l = 0xdead;
      }
      else if ( atom == 'D' || atom == 'd' ){
        mode = 2;
        l = 12345678;
      }
      else {
        return Error(SYNTAX_ERR, __FUNCTION__ );
      }
      GetAtom();

      if ( GetLabel( &label) ) return 1;

      FindLabel( &label, &l );

      if ( mode == 1  ){
        fprintf(my_stderr,"$%x",l);
      }
      else if ( mode == 2 ){
        fprintf(my_stderr,"%d",l);
      }
    }
    else {
      fputc(atom,my_stderr);
      GetAtom();
    }
  }
  fputc('\n',my_stderr);

  if ( d ) return Error(FAIL_ERR,"");

  return 0;

}
/*

  REPT

*/
int p_rept(int d)
{
  int32_t l;

  if ( Current.rept ) return Error(REPT1_ERR,"");

  if ( NeedConst( &l , "REPT") ) return 1;

  if ( l <= 0 ) return Error(REPT2_ERR,"");

  Current.reptValue = l;
  Current.reptStart = Current.SrcPtr;
  Current.reptLine = Current.Line;
  Current.rept++;
  return 0;
}

int p_endr(int d)
{
  if ( ! Current.rept ) return Error(REPT2_ERR,"");

  if ( --Current.reptValue ){
    Current.SrcPtr = Current.reptStart;
    Current.Line = Current.reptLine;
  } else {
    Current.rept--;
  }

  return 0;
}
/*

  LIST

*/
int p_list(int d)
{
  int32_t l;
  if ( NeedConst( &l , "LIST" ) ) return 1;

  if ( l <= 0 ) l = 0;
  if ( l > 3 ) l = 3;
  verbose = l;

  return 0;
}
/*

  ISYMS

*/
int p_isyms(int d)
{
  return Error(-ISYMS_ERR,"ISYMS is no longer supported");
}
/*

  GLOBAL

*/
int p_global(int d)
{
  label_t label;
  label_t *plabel;
  int32_t l;

  do{

    if ( GetLabel( &label ) ) return 1;

    if ( (plabel = FindLabel( &label, &l)) == NULL ){
      int solved;

      label.type = GLOBAL|UNSOLVED;
      label.value = Global.pc;
      DefineLabel( &label, &solved);
    } else {
      if ( plabel->type & LOCAL ) return Error(SYNTAX_ERR,"");

      plabel->type |= GLOBAL;
    }

  }while (TestAtom(','));

  return 0;
}

int p_mode(int modus)
{
  sourceMode = modus;
  if ( modus == JAGUAR_GPU ){
    jaguar_mode = 0;
  } else  if ( modus == JAGUAR_DSP ){
    jaguar_mode = 1;
  } else {
    jaguar_mode = -1;
  }
  //  printf("Switching to mode %d\n",modus);
  return 0;
}

int p_regtop(int dummy)
{
  int32_t l;
  if ( sourceMode == LYNX ) {
    return Error(SYNTAX_ERR,"REGTOP only in RISC modes");
  }
  if ( NeedConst( &l, "REGTOP") ) return 1;

  if ( (l < 0 || l > 31) ) Error(REG_ERR,"");

  risc[jaguar_mode].regAllocStart = l;

  return 0;
}

int p_regmap(int dummy)
{
  int i;
  (void)dummy;
  if ( sourceMode == LYNX ) {
    return Error(SYNTAX_ERR,"REGMAP only in RISC modes");
  }
  if ( jaguar_mode == 0 ){
    fprintf(my_stderr,"Reg %-32s %-32s\n","Bank0 - GPU","Bank 1 - GPU");
  } else {
    fprintf(my_stderr,"Reg %-32s %-32s\n","Bank0 - DSP","Bank 1 - DSP");
  }
  for(i = 31; i >= 0; --i ){
    if ( risc[jaguar_mode].reg_flag[i] || risc[jaguar_mode].reg_flag[i+32] ){
      fprintf(my_stderr," %2d ",i);
      if ( risc[jaguar_mode].reg_flag[i] ){
        fprintf(my_stderr, "%-32s ",risc[jaguar_mode].reg_label[i]->name);
      } else {
        fprintf(my_stderr, "%-32s "," ");
      }
      if ( risc[jaguar_mode].reg_flag[i+32] ){
        fprintf(my_stderr, "%-32s ",risc[jaguar_mode].reg_label[i+32]->name);
      } else {
        fprintf(my_stderr, "%-32s "," ");
      }
      fprintf(my_stderr, "\n");
    }
  }
  return 0;
}

int p_reg(int d)
{
  int32_t l;
  int i,o;
  int allowRedefine;

  if ( sourceMode == LYNX ) {
    return Error(SYNTAX_ERR,"REG only in RISC modes");
  }

  if ( ! Current.Label.len ) Error(SYNTAX_ERR,"");

  if ( TestAtomOR('r','R') ){
    if ( getdec32( &l ) == EXPR_ERR ) return Error(SYNTAX_ERR,"");
    if ( l > 31 ) return Error(SYNTAX_ERR,"");
  } else {
    if ( NeedConst( &l, "REG") ) return 1;
  }

  if ( (l < 0 || l > 31) && l != 99 ) Error(REG_ERR,"");

  if ( (Current.LabelPtr->type == REGISTER ) &&
       (Current.LabelPtr->file != -1) ) return Error(REG1_ERR,"");

  if ( Current.LabelPtr->type != NORMAL &&
       Current.LabelPtr->type != REGISTER ) return Error(SYNTAX_ERR,"");

  o = l;
  i = (int)strlen(Current.LabelPtr->name);
  if ( i > 2 &&
       Current.LabelPtr->name[i-2] == '.' &&
       Current.LabelPtr->name[i-1] == 'a'){
    o += 32;
  }
  allowRedefine = TestAtom('!');
  if ( (o == 99 || o == (99+32)) && allowRedefine ){
    return Error(SYNTAX_ERR,
                 "'!' not allowed with automatic register allocation.");
  }
  if ( o == 99 || o == (99+32) ){
    int end = o-99;
    i = risc[jaguar_mode].regAllocStart;
    if ( o == 99+32 ){
      i+=32;
    }
    o = -1;
    for( ; i >= end; --i){
      if ( risc[jaguar_mode].reg_flag[i] == 0 ){
        o = i;
        l = o & 31;
        break;
      }
    }
    if ( o == -1 ){
      return Error(SYNTAX_ERR,"No more free register");
    }
  }

  if ( risc[jaguar_mode].reg_flag[o] && allowRedefine == 0 ) {
    char help[256];
    sprintf(help,"Register already defined: %s:%5d",
            file_list[risc[jaguar_mode].reg_file[o]].name,
            risc[jaguar_mode].reg_line[o]);

    Warning(help);
  }

  risc[jaguar_mode].reg_flag[o] = 1;
  risc[jaguar_mode].reg_file[o] = Current.File;
  risc[jaguar_mode].reg_line[o] = Current.Line;
  risc[jaguar_mode].reg_label[o] = Current.LabelPtr;
  Current.LabelPtr->file = Current.File;
  Current.LabelPtr->type = REGISTER;
  Current.LabelPtr->value = l;
  // Current.varModifier++;
  // printf("(%s)%d\n",Current.LabelPtr->name,Current.LabelPtr->value);
  return 0;
}
/*

  UNREG

*/
int p_unreg(int d)
{
  label_t label;
  label_t *plabel;
  int32_t l;

  do{
    if ( GetLabel( &label ) ) return 1;

    if ( (plabel = FindLabel( &label, &l)) == NULL ){
      return Error(SYNTAX_ERR,"");
    } else {
      int i,o;

      //      printf("UNREG : %s\n",plabel->name);

      if ( !(plabel->type & REGISTER) ) return Error(SYNTAX_ERR,"");

      if ( plabel->file == -1 ){
        char help[80];
        sprintf(help,"Multiple UNREG on (%s)!",plabel->name);
        Warning(help);
      }

      plabel->file = -1;
      plabel->value = -1;
      o = l;
      i = (int)strlen(plabel->name);
      if ( i > 2 &&
           plabel->name[i-2] == '.' &&
           plabel->name[i-1] == 'a'){
        o += 32;
      }
      risc[jaguar_mode].reg_flag[o] = 0;
      risc[jaguar_mode].reg_line[o] = 0;
      risc[jaguar_mode].reg_file[o] = 0;
      risc[jaguar_mode].reg_label[o] = NULL;
    }

  }while (TestAtom(','));

  return 0;
}
/*---------------------------------------------------------------------------*/
int p_macmode(int d)
{
  mac_mode = d;
  return 0;
}
/*---------------------------------------------------------------------------*/

int CheckPseudo(char *s)
{
  int (*fun)(int );
  int para;

  if ( SearchOpcode2(pseudo, s, &fun, &para) < 0 )
    return -1;

  if ( Current.Macro.Define ){
    if ( (fun == p_macro) || (fun == p_endm) ) return fun(para);
    killLine();
    return 0;
  }

  if ( !Current.ifFlag ){
    if ( ((fun == p_if) || (fun == p_ifdef) ||
          (fun == p_ifundef) || (fun == p_ifvar)) ||
         ((fun == p_else) || (fun == p_endif)) )
    {
      return fun(para);
    }
    killLine();
    return 0;
  }

  if ( !Current.switchFlag ){
    if ( (fun == p_switch) || (fun == p_case) ||
         (fun == p_default) || (fun == p_ends) )
    {
      return fun(para);
    }
    killLine();
    return 0;
  }

  return fun(para);

}
