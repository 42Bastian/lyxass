/*
    mnemonics.c

    Handle 65C02 mnemonics

*/

#include <stdio.h>

#include "my.h"
#include "error.h"
#include "label.h"
#include "global_vars.h"
#include "parser.h"
#include "output.h"
#include "opcode.h"
#include "mnemonics.h"
#include "jaguar.h"

// ref.c
void saveCurrentLine();

// this must be provided by any mnemonic.c file
int Endian(void)
{
  if ( sourceMode == LYNX ){
    return targetLITTLE_ENDIAN;
  } else {
    return targetBIG_ENDIAN;
  }
}

// one-byte commands
int op0(int op)
{
  writeConstByte( (char) op );
  CYCLES += 2;
  if ( op == 0x48 || op == 0xda || op == 0x5a || op == 0x08 ) ++CYCLES;
  if ( op == 0x68 || op == 0xfa || op == 0x7a || op == 0x28 ) CYCLES += 2;
  if ( op == 0x40 || op == 0x66 ) CYCLES += 6;
  return 0;
}

// branch
int op1(int op)
{
  int32_t l;
  int err;

  if ( (err = Expression(&l)) == EXPR_ERR) return 1;

  if ( err == EXPR_OK ){
    int dist = l - Global.pc - 2;
    if ( dist > 127 || dist < -128 )return Error(DISTANCE_ERR,"");
    writeConstByte(op);
    writeConstByte((char)dist);
  } else {
    saveCurrentLine();
    writeConstByte(op);
    writeConstByte(0);
  }
  CYCLES += 2;
  if ( (l & ~255) != ( Global.pc & ~255) ) ++CYCLES;
  return 0;
}

// jsr => absolute adress

int op2(int op)
{
  int32_t l;
  int err;

  if ( (err = Expression(&l)) == EXPR_ERR) return 1;
  if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");

  writeConstByte(op);
  writeWordLittle((short) l);

  if ( err == EXPR_UNSOLVED ) saveCurrentLine();

  CYCLES += 6;

  return 0;
}

// ora,and,eor,adc,sta,lda,cmp,sbc

int op3(int op)
{
  int err;
  int32_t l;
  int komma = 0;
  int saveCYCLES;
  int saveOp;

  if ( ! KillSpace() ){
    return Error(SYNTAX_ERR,"");
  }

  // immediate

  if ( TestAtom('#') ){

    if ( (err = Expression(&l)) == EXPR_ERR) return 1;

    if ( l < -128 || l > 255 ) return Error(BYTE_ERR,"");
    if ( op == 0x81 ) return Error(SYNTAX_ERR,""); // STA #0 not possible
    writeConstByte(op | 0x08);
    if ( Current.needsReloc ){
      writeRelocByte((char)l,RELOC_LOBY);
    } else {
      writeConstByte((char)l);
    }
    if ( err == EXPR_UNSOLVED ) saveCurrentLine();

    CYCLES += 2;

    if ( (Global.pc & 255) == 1) ++CYCLES;

    return 0;
  }

  // indirect

  SavePosition();
  saveCYCLES = CYCLES;
  saveOp = op;
  if ( TestAtom('(') ){

    if ( (err = Expression(&l)) == EXPR_ERR) return 1;
    if ( l < 0 || l > 255 ) return Error(BYTE_ERR,"");

    if ( TestAtom(',') ){
      if ( !TestAtomOR('x','X') ) return Error(SYNTAX_ERR,"");
      komma = 1;
      CYCLES += 6;
    }
    if ( !TestAtom(')') ) return Error(SYNTAX_ERR,"Missing ')'");

    if ( !komma ) {
      if ( TestAtom(',') ){
	CYCLES += 5;
	if ( !TestAtomOR('y','Y') ) return Error(SYNTAX_ERR,"");
	op |= 0x10;
      } else {
	op |= 0x12;
	--op;
	CYCLES++;
      }
    }
    KillSpace();
    if ( atom == 0 ){
      if ( op == 0x91 ) ++CYCLES;
      writeConstByte(op);
      writeByte((char)l);
      if ( err == EXPR_UNSOLVED ) saveCurrentLine();
      return 0;
    } else {
      // Possible not indirect but expression with braces
      RestorePosition();
      op = saveOp;
      CYCLES = saveCYCLES;
    }
  }

  // absolute

  if ( (err = Expression(&l)) == EXPR_ERR) return 1;

  if ( TestAtom(',') ){
    CYCLES += 4;

    if ( TestAtomOR('X','x') ){
      op |= 0x14;
    } else if ( TestAtomOR('Y','y') ) {
      op |= 0x18;
      writeConstByte(op);
      writeWordLittle((short)l);
      if ( err == EXPR_UNSOLVED ) saveCurrentLine();
      return 0;
    } else return Error(SYNTAX_ERR,"");
  } else {
    op |= 0x04;
  }

  if ( l < 0 || l > 65535 ) Error(WORD_ERR,"");

  if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2) {
    CYCLES+=4;
    op |= 0x08;
    writeConstByte(op);
    writeWordLittle((short)l);
  } else {
    CYCLES+=3;
    writeConstByte(op);
    writeByte((char)l);
  }
  if ( op == 0x99 || op == 0x9d ) ++CYCLES;

  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}

// trb,tsb

int op4(int op)
{
  int err;
  int32_t l;

  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;

  if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");

  if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
    CYCLES += 6;
    writeConstByte(op | 0x08);
    writeWordLittle((short)l);
  } else {
    CYCLES += 5;
    writeConstByte(op);
    writeByte((char)l);
  }
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}

// stz

int op5(int op)
{
  int err;
  int32_t l;
   int brace;

  KillSpace();
  brace = atom == '(';

  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
  if ( brace && last_atom == ')' ){
    Warning("STZ (n) translates to STZ n\n");
  }

  if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");

  if ( TestAtom(',') ) {

    if ( !TestAtomOR('x','X') ) Error(SYNTAX_ERR,"");

    if (l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
      writeConstByte( op | 0x9a );
      writeWordLittle((short)l);
      CYCLES += 5;
    } else {
      writeConstByte( op | 0x70 );
      writeByte((char)l);
      CYCLES += 4;
    }
    if ( err == EXPR_UNSOLVED ) saveCurrentLine();
    return 0;
  }

  if (l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
    writeConstByte( op | 0x98 );
    writeWordLittle((short)l);
    CYCLES += 4;
  } else {
    writeConstByte( op | 0x60 );
    writeByte((char)l);
    CYCLES += 3;
  }
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}

// stx
int op6(int op)
{
  int err;
  int32_t l;
  int brace;

  KillSpace();
  brace = atom == '(';
  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
  if ( brace && last_atom == ')' ){
    Warning("STX (n) translates to STX n\n");
  }

  if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");

  if ( TestAtom(',') ) {
    if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ) return Error(BYTE_ERR,"");

    if ( !TestAtomOR('y','Y') ) Error(SYNTAX_ERR,"");
    CYCLES += 4;
    writeConstByte(op | 0x10);
    writeByte((char)l);
  } else {
    if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
      CYCLES += 4;
      writeConstByte(op | 0x08 );
      writeWordLittle((short)l);
    } else {
      CYCLES += 3;
      writeConstByte(op);
      writeByte((char)l);
    }
  }

  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}

// sty

int op7(int op)
{
  int err;
  int32_t l;
  int brace;

  KillSpace();
  brace = atom == '(';

  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
  if ( brace && last_atom == ')' ){
    Warning("STY (n) translates to STY n\n");
  }

  if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");

  if ( TestAtom(',') ) {

    if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ) return Error(BYTE_ERR,"");

    if ( !TestAtomOR('x','X') ) Error(SYNTAX_ERR,"");
    CYCLES += 4;
    writeConstByte(op | 0x10);
    writeByte((char)l);
  } else {
    if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
      CYCLES += 4;
      writeConstByte(op | 0x08 );
      writeWordLittle((short)l);
    } else {
      CYCLES += 3;
      writeConstByte(op);
      writeByte((char)l);
    }
  }
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}

// bit,ldy,cpy

int op8(int op)
{
  int err;
  int32_t l;
  int flag = 0;
  int brace = 0;

  if ( TestAtom('#') ){

    if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
    if ( l < -128 || l > 255 ) return Error(BYTE_ERR,"");
    CYCLES += 2;
    writeConstByte(op == 0x20 ? 0x89 : op);
    writeByte((char)l);
  } else {
    brace = atom == '(';
    if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
    if ( brace && last_atom == ')' ){
      Warning("BIT/LDY/CPY (n) translates to BIT/LDY/CPY n\n");
    }

    if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");
    op |= 0x04;
    CYCLES += 3;
    if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
      op |= 0x08;
      flag = 1;
      CYCLES++;
    }
    if ( TestAtom(',') ){
      if ( (op & 0xc0) == 0xc0) return Error(SYNTAX_ERR,"");                     // no cpy ABCD,x !!!
      if ( !TestAtomOR('x','X') ) return Error(SYNTAX_ERR,"");
      op |= 0x10;
      CYCLES++;
    }

    writeConstByte(op);
    if ( flag ){
      writeWordLittle((short)l);
    } else {
      writeByte((char)l);
    }
  }
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;

}
/*
  ldx
*/
int op9(int op)
{
  int err;
  int32_t l;
  int flag = 0;
  int brace = 0;

  if ( TestAtom('#') ){

    if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
    if ( l < -128 || l > 255 ) return Error(BYTE_ERR,"");
    CYCLES += 2;
    writeConstByte(op);
    writeByte((char)l);
  } else {
    brace = atom == '(';
    if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
    if ( brace && last_atom == ')' ){
      Warning("LDX (n) translates to LDX n\n");
    }

    if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");
    CYCLES += 3;
    op |= 0x04;
    if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
      op |= 0x08;
      flag = 1;
      ++CYCLES;
    }
    if ( TestAtom(',') ){
      if ( !TestAtomOR('y','Y') ) return Error(SYNTAX_ERR,"");
      op |= 0x10;
      ++CYCLES;
    }

    writeConstByte(op);
    if ( flag ){
      writeWordLittle((short)l);
    } else {
      writeByte((char)l);
    }
  }
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}

/* asl/rol/lsr/ror/dec/inc */

int opa(int op)
{
  int err;
  int32_t l;

  KillSpace();
  if ( !atom ){
    op |= 8;
    if ( op >= 0xca ) op ^= 0xf0;
    writeConstByte(op);
    CYCLES += 2;
    return 0;
  }

  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
  if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");

  CYCLES += 5;
  op |= 0x04;
  if ( l > 255 || err == EXPR_UNSOLVED || Current.pass2 ){
    ++CYCLES;
    op |= 0x0a;
    if ( TestAtom(',') ){
      if ( !TestAtomOR('x','X') ) return Error(SYNTAX_ERR,"");
      op |= 0x10;
    }
    writeConstByte(op);
    writeWordLittle((short)l);
  } else {
    if ( TestAtom(',') ){
      if ( !TestAtomOR('x','X') ) return Error(SYNTAX_ERR,"");
      op |= 0x10;
      ++CYCLES;
    }
    writeConstByte(op);
    writeByte((char)l);
  }
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}
/*
  rmb / smb
*/
int opb(int op)
{
  int err;
  int32_t l;

  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
  if ( l < 0 || l > 255 ) return Error(BYTE_ERR,"");

  writeConstByte(op);
  writeByte((char)l);

  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}
/*
  JMP
*/
int opc(int op)
{
  int err;
  int32_t l;
  int saveCYCLES;
  int saveOp;

  saveOp = op;
  saveCYCLES = CYCLES;
  SavePosition();

  if ( TestAtom('(') ){
    if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
    if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");
    op |= 0x20;
    if ( TestAtom(',') ){
      if ( !TestAtomOR('x','X') ) return Error(SYNTAX_ERR,"");
      op |= 0x10;
    }
    if ( !TestAtom(')') ) return Error(SYNTAX_ERR,"");
    KillSpace();
    if ( atom == 0 ){
      CYCLES += 6;
      writeConstByte(op);
      writeWordLittle((short)l);
      if ( err == EXPR_UNSOLVED ) saveCurrentLine();
      return 0;
    } else {
      // Possible not indirect but expression with braces
      op = saveOp;
      CYCLES = saveCYCLES;
      RestorePosition();
    }
  }
  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
  if ( l < 0 || l > 65535 ) return Error(WORD_ERR,"");
  if ( err != EXPR_UNSOLVED ){
    int dist = Global.pc - l - 2;
    if ( dist >= -128 && dist <= 127 ) Warning("JMP can be changed into BRA !\n");
  }
  CYCLES += 3;

  writeConstByte(op);
  writeWordLittle((short)l);
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  return 0;
}
// brk
// this opcode has an immediate-value on the lynx
int opd(int op)
{
  int err;
  int32_t l;

  if ( !TestAtom('#') ) return Error(SYNTAX_ERR,"");

  if ( (err = Expression( &l )) == EXPR_ERROR ) return 1;
  if ( l < -128 || l > 255 ) return Error(BYTE_ERR,"");
  writeConstByte(op);
  writeByte((char)l);
  if ( err == EXPR_UNSOLVED ) saveCurrentLine();
  CYCLES += 7;
  return 0;
}

// BBRx,BBSx
int ope(int op)
{
  int32_t l;
  int32_t dst;
  int err_l;
  int err_dst;

  err_l = Expression(&l);

  if ( err_l == EXPR_ERR) return 1;

  if ( l < 0 || l > 255 ) return Error(BYTE_ERR,"");

  if ( !TestAtom(',') ) return Error(SYNTAX_ERR,"");

  err_dst = Expression(&dst);
  if ( err_dst == EXPR_OK ){
    int dist = dst - Global.pc - 3;
    if ( dist > 127 || dist < -128 )return Error(DISTANCE_ERR,"");
    writeConstByte(op);
    writeByte((char)l);
    writeConstByte((char)dist);
  } else {
    saveCurrentLine();
    writeConstByte(op);
    writeByte((char)l);
    writeConstByte(0);
  }

  CYCLES += 5;

  if ( err_l == EXPR_UNSOLVED && err_dst != EXPR_UNSOLVED ) saveCurrentLine();

  return 0;
}

int CheckMnemonic(char *s)
{
  if ( Current.ifFlag && Current.switchFlag){
    if ( sourceMode == LYNX ){
      if (SearchOpcode(OpCodes,s) < 0 ) return -1;
    } else {
      if (SearchOpcode(JaguarOpcodes,s) < 0 ) return -1;
    }

    if ( GetComment() ){
      Error(GARBAGE_ERR, srcLinePtr);
      return 1;
    }
  }

  return 0;
}
