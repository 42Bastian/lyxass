/* mnemonics.h
 * created 20.03.96
 * (c) Bastian Schick
 *
 */

//#include "opcode.h"
int direct(int );
int one_reg(int );
int two_reg(int );
int imm_reg(int );  // immediate 1..32
int imm2_reg(int ); // immediate 0..31
int imm3_reg(int ); // immediate -16..15
int imm4_reg(int ); // immediate long
int cond_rel(int ); // condition, relative
int cond_abs(int ); // condition, absolute
int load_reg(int );
int load2_reg(int );
int store_reg(int );
int store2_reg(int );

const struct opcode_s JaguarOpcodes[] = {
  { "ABS", one_reg, 22 },
  { "ADD", two_reg, 0 },
  { "ADDC", two_reg, 1},
  { "ADDQ", imm_reg, 2},
  { "ADDQT", imm_reg, 3},
  { "ADDQMOD", imm_reg,63},
  { "AND", two_reg, 9},
  { "BCLR", imm2_reg, 15},
  { "BSET", imm2_reg, 14},
  { "BTST", imm2_reg, 13},
  { "CMP", two_reg,30},
  { "CMPQ", imm3_reg,31},
  { "DIV", two_reg,21},
  { "IMACN", two_reg,20},
  { "IMULT", two_reg, 17},
  { "IMULTN", two_reg,18},
  { "JR", cond_rel,53},
  { "JUMP", cond_abs,52},
  { "LOAD", load_reg, 41},
  { "LOADB", load2_reg, 39},
  { "LOADW", load2_reg, 40},
  { "LOADP", load2_reg, 42},
  { "MMULT", two_reg,54},
  { "MOVE", two_reg,34},
  { "MOVEFA", two_reg,37},
  { "MOVETA", two_reg,36},
  { "MOVEI", imm4_reg,38},
  { "MOVEQ", imm2_reg,35},
  { "MTOI", two_reg, 55},
  { "MULT", two_reg, 16},
  { "MIRROR", one_reg, 48},
  { "NEG", one_reg, 8},
  { "NOP", direct, 57},
  { "NORMI",two_reg,56},
  { "NOT", one_reg, 12},
  { "OR", two_reg, 10},
  { "PACK", one_reg, 63},
  { "RESMAC", one_reg, 19},
  { "ROR", two_reg, 28},
  { "RORQ", imm_reg, 29},
  { "SAT8", one_reg, 32},
  { "SAT16", one_reg, 33},
  { "SAT24", one_reg, 62},
  { "SAT16S", one_reg, 33},
  { "SAT32S", one_reg,42},
  { "SH", two_reg, 23},
  { "SHA", two_reg, 26},
  { "SHARQ", imm_reg,27},
  { "SHLQ", imm_reg,24},
  { "SHRQ", imm_reg,25},
  { "STORE", store_reg,47},
  { "STOREB", store2_reg, 45},
  { "STOREP", store2_reg, 48},
  { "STOREW", store2_reg, 46},
  { "SUB", two_reg, 4},
  { "SUBC",two_reg,5},
  { "SUBQ",imm_reg,6},
  { "SUBQT", imm_reg,7},
  { "SUBQMOD", imm_reg,32},
  { "UNPACK", one_reg,64},
  { "XOR",two_reg,11},
  { "\0,",0,0}
};
