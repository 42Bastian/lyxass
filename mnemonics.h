/* mnemonics.h
 * created 20.03.96
 * (c) Bastian Schick
 *
 */

/*****************/
/* address-modes */
/*****************/
#define OP0		0	/* implied */
#define OP1		1	/* relative branches */
#define OP2		2	/* jsr */
#define OP3		3	/* ora-group */
#define OP4		4	/* tsb/trb */
#define OP5		5	/* stz */
#define OP6		6	/* stx */
#define OP7		7	/* sty */
#define OP8		8	/* bit/ldy/cpy */
#define OP9		9	/* cpx,ldx */
#define OPA		10	/* asl/rol/lsr/ror/dec/inc */
#define OPB		11	/* rmb/smb */
#define OPC		12	/* jmp */
#define OPD		13	/* brk */
#define OPE		14	/* bbr/bbs */
#define OPF		15	/* pseudo - opcode */

int op0(int);
int op1(int);
int op2(int);
int op3(int);
int op4(int);
int op5(int);
int op6(int);
int op7(int);
int op8(int);
int op9(int);
int opa(int);
int opb(int);
int opc(int);
int opd(int);
int ope(int);


const struct opcode_s OpCodes[] = {
  {"ADC" ,op3,0x61},
  {"AND" ,op3,0x21},
  {"ASL" ,opa,0x02},
  {"BBR0",ope,0x0F},
  {"BBR1",ope,0x1F},
  {"BBR2",ope,0x2F},
  {"BBR3",ope,0x3F},
  {"BBR4",ope,0x4F},
  {"BBR5",ope,0x5F},
  {"BBR6",ope,0x6F},
  {"BBR7",ope,0x7F},
  {"BBS0",ope,0x8F},
  {"BBS1",ope,0x9F},
  {"BBS2",ope,0xAF},
  {"BBS3",ope,0xBF},
  {"BBS4",ope,0xCF},
  {"BBS5",ope,0xDF},
  {"BBS6",ope,0xEF},
  {"BBS7",ope,0xFF},
  {"BCC" ,op1,0x90},
  {"BLT" ,op1,0x90},
  {"BCS" ,op1,0xB0},
  {"BGE" ,op1,0xB0},
  {"BEQ" ,op1,0xF0},
  {"BIT" ,op8,0x20},
  {"BMI" ,op1,0x30},
  {"BNE" ,op1,0xD0},
  {"BPL" ,op1,0x10},
  {"BRA" ,op1,0x80},
  {"BRK" ,opd,0x00},
  {"BVC" ,op1,0x50},
  {"BVS" ,op1,0x70},
  {"CLC" ,op0,0x18},
  {"CLD" ,op0,0xD8},
  {"CLI" ,op0,0x58},
  {"CLV" ,op0,0xB8},
  {"CMP" ,op3,0xC1},
  {"CPX" ,op9,0xE0},
  {"CPY" ,op8,0xC0},
  {"DEC" ,opa,0xc2}, {"DEA", opa, 0x3a},
  {"DEX" ,op0,0xca},
  {"DEY" ,op0,0x88},
  {"EOR" ,op3,0x41},
  {"INC" ,opa,0xE2},   {"INA" ,opa,0x1a},
  {"INX" ,op0,0xe8},
  {"INY" ,op0,0xc8},
  {"JMP" ,opc,0x4c},
  {"JSR" ,op2,0x20},
  {"LDA" ,op3,0xA1},
  {"LDX" ,op9,0xA2},
  {"LDY" ,op8,0xA0},
  {"LSR" ,opa,0x42},
  {"NOP" ,op0,0xEA},
  {"ORA" ,op3,0x01},
  {"PHA" ,op0,0x48},
  {"PHP" ,op0,0x08},
  {"PHX" ,op0,0xDA},
  {"PHY" ,op0,0x5a},
  {"PLA" ,op0,0x68},
  {"PLP" ,op0,0x28},
  {"PLX" ,op0,0xfa},
  {"PLY" ,op0,0x7a},
 /****************/
  {"RMB0",opb,0x07},
  {"RMB1",opb,0x17},
  {"RMB2",opb,0x27},
  {"RMB3",opb,0x37},
  {"RMB4",opb,0x47},
  {"RMB5",opb,0x57},
  {"RMB6",opb,0x67},
  {"RMB7",opb,0x77},
 /*****************/
  {"ROL", opa,0x22},
  {"ROR", opa,0x62},
  {"RTI", op0,0x40},
  {"RTS", op0,0x60},
  {"SBC", op3,0xE1},
  {"SEC", op0,0x38},
  {"SED", op0,0xf8},
  {"SEI", op0,0x78},
 /****************/
  {"SMB0",opb,0x87},
  {"SMB1",opb,0x97},
  {"SMB2",opb,0xA7},
  {"SMB3",opb,0xB7},
  {"SMB4",opb,0xC7},
  {"SMB5",opb,0xD7},
  {"SMB6",opb,0xE7},
  {"SMB7",opb,0xF7},
  /*****************/
  {"STA" ,op3,0x81},
  {"STX" ,op6,0x86},
  {"STY" ,op7,0x84},
  {"STZ" ,op5,0x04},
  {"TAX" ,op0,0xAA},
  {"TAY" ,op0,0xA8},
  {"TRB" ,op4,0x14},
  {"TSB" ,op4,0x04},
  {"TSX" ,op0,0xBA},
  {"TXA" ,op0,0x8a},
  {"TXS" ,op0,0x9a},
  {"TYA" ,op0,0x98},
  {"\0" ,0,0}
  };

/*****/
/*EOF*/
/*****/
