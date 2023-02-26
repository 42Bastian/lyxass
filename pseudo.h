/*

  def. of pseudo-op-codes

*/

/*
  opcodes must not be longer than 7 char's !
*/

#include "opcode.h"

int p_org(int);
int p_run(int);
int p_end(int);
int p_setvar(int);
int p_incvar(int);
int p_macro(int);
int p_endm(int);
int p_definebyte(int);
int p_defineword(int);
int p_definelong(int);
int p_definephrase(int);
int p_defineascii(int);
int p_definespace(int);
int p_include(int);
int p_isyms(int);
int p_trans(int);
int p_path(int);
int p_equ(int);
int p_set(int);
int p_if(int);
int p_ifdef(int);
int p_ifundef(int);
int p_ifvar(int);
int p_else(int);
int p_endif(int);
int p_switch(int);
int p_case(int);
int p_default(int);
int p_ends(int);
int p_align(int);
int p_echo(int);
int p_rept(int);
int p_endr(int);
int p_ibytes(int);
int p_list(int);
int p_global(int);
int p_mode(int);
int p_reg(int);
int p_unreg(int);
int p_macmode(int);
int p_regmap(int);
int p_regtop(int);

const struct opcode_s pseudo[]={
    { "ORG",p_org,0 },
    { "RUN",p_run,0 },
    { "END",p_end,0 },
    { "SETVAR",p_setvar,0},{ "SET@",p_setvar,0},
    { "INCVAR",p_incvar,1},{ "INC@",p_incvar,1},
    { "DECVAR",p_incvar,-1},{ "DEC@",p_incvar,-1},
    { "MACRO",p_macro,0},
    { "ENDM",p_endm,0},
    { "DB",p_definebyte,0},{"DC.B",p_definebyte,0},
    { "DA",p_definebyte,1},{"DC.A",p_definebyte,1},
    { "DW",p_defineword,0},{"DC.W",p_defineword,0},
    { "DW",p_definelong,0},{"DC.L",p_definelong,0},
    { "DP",p_definephrase,0},{"DC.P",p_definephrase,0},
    { "DS.B",p_definespace,1},{"DS",p_definespace,1},
    { "DS.W",p_definespace,2},
    { "DS.L",p_definespace,4},
    { "DS.P",p_definespace,8},
    { "INCLUDE",p_include,0},
    { "ISYMS",p_isyms,0},
    { "TRANS",p_trans,0},
    { "PATH",p_path,0},
    { "EQU",p_equ,0},
    { "SET",p_set,0},
    { "IF",p_if,0},
    { "IFD",p_ifdef,0},{"IFDEF",p_ifdef,0},
    { "IFND",p_ifundef,0},{"IFUNDEF",p_ifundef,0},
    { "IFVAR",p_ifvar,0},
    { "ELSE",p_else,0},
    { "ENDIF",p_endif,0},
    { "SWITCH",p_switch,0},
    { "CASE",p_case,0},
    { "DEFAULT",p_default,0},{ "ELSES",p_default,0},
    { "ENDS",p_ends,0},
    { "ALIGN",p_align,0},
    { "LONG",p_align,4},{ "PHRASE",p_align,8},{ "DPHRASE",p_align,16},
    { "QPHRASE",p_align,32},
    { "ECHO",p_echo,0}, { "FAIL",p_echo,1},
    { "REPT",p_rept,0},
    { "ENDR",p_endr,0},
    { "IBYTES",p_ibytes,0},{ "INLINE",p_ibytes,0},    { "INCBIN",p_ibytes,0},
    { "LIST",p_list,0},
    { "GLOBAL",p_global,0},{ "GLOBL",p_global,0},
    { "AGPU",p_mode,JAGUAR_GPU},{"ADSP",p_mode,JAGUAR_DSP},{"LYNX",p_mode,LYNX},
    { "GPU",p_mode,JAGUAR_GPU},{"DSP",p_mode,JAGUAR_DSP},{"_6502",p_mode,LYNX},
    { "REG",p_reg,0}, { "UNREG",p_unreg,0},
    { "EQUR",p_reg,1},{ "EQURUNDE",p_unreg,1},
    { "REGMAP", p_regmap,0},
    { "REGTOP", p_regtop,0},
//->    { "MAC",p_macmode,1},{ "LYXASS",p_macmode,0},
    { "\0",0,0 }
};
