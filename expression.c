/*

  expression.c


  Expression-parser



*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "my.h"
#include "label.h"
#include "error.h"
#include "global_vars.h"
#include "parser.h"

int Expression( long * value);
int sum(long *value);

int ExprFailed;

/* returns:

   0 => expression ok => value is set
   1 => error in Expression
   2 => expression could not completly resolved, try later

*/


#define white(c) (((c) == ' ') || ((c) == '\t'))

// temp.

static LABEL expr_label;

#define need_ch()\
    if ( ! GetAtom() ){\
      Error(EXPR_ERR,"");\
      return EXPR_ERR;\
    }


int getdec(long *value)
{
  long dec = 0;
  int i = 0;

  while ( isdigit(atom) ){
    dec *= 10;
    dec += atom - '0';
    GetAtom();
    ++i;
  }

  if ( !i ){
    Error(EXPR_ERR,"");
    return EXPR_ERR;
  }

  if ( (dec < 0) || (i > 10) ){
    *value = 0;
    Error(TOOBIG_ERR,"");
    return EXPR_ERROR;
  }

  *value = dec;
  return EXPR_OK;
}

int gethex(long *value)
{
  long hex = 0;
  int i = 0;

  while ( (atom >= '0' && atom <= '9') ||
	  (atom >= 'A' && atom <= 'F') ||
	  (atom >= 'a' && atom <= 'f') ){
    hex <<= 4;
    if ( atom > '9' )
      hex += ( atom & 0xdf ) - '0' - 7;
    else
      hex += atom - '0';

    GetAtom();
    ++i;
  }

  if ( !i ){
    Error(EXPR_ERR,"");
    return EXPR_ERR;
  }

  if ( (i > 8) ){
    *value = 0;
    Error(TOOBIG_ERR,"");
    return EXPR_ERROR;
  }
  *value = hex;
  return EXPR_OK;
}

int getbin(long *value)
{
  long bin = 0;
  int i = 0;
  while ( atom == '1' || atom == '0' ){
    bin <<= 1;
    bin += (atom - '0');
    GetAtom();
    ++i;
  }

  if ( !i ){
    Error(EXPR_ERR,"");
    return EXPR_ERR;
  }

  if ( i > 32 ){
    Error(TOOBIG_ERR,"");
    return EXPR_ERROR;
  }
  *value = bin;
  return EXPR_OK;
}

int getlabel(long *value)
{
  long lv = 0;
  int dummy;

  if ( GetLabel( &expr_label) ) return EXPR_ERROR;

  expr_label.type &= ~UNSURE;

  // printf("Type %02x\n",expr_label.type);

  if ( FindLabel( &expr_label, &lv ) ){
    if (expr_label.type & CODELABEL){
      ++Current.needsReloc;
//->      printf("Reloc: %s Line %4d %04x: %04x\n",file_list[Current.File].name,Current.Line,Global.pc,lv);
    }
    *value = lv;
    return EXPR_OK;
  }

  if ( expr_label.type & UNSOLVED ){
    unknownLabel = DefineLabel( &expr_label, &dummy);
  } else {
    expr_label.type |= UNSOLVED;
    unknownLabel = DefineLabel( &expr_label, &dummy);
  }

  *value = 0;
  ExprFailed = 1;
  return EXPR_OK;
}

int uni(long *value)
{
  int err;
  char save;
  long a = 0;


  KillSpace();

  save = atom;

  if ( atom == '-' || atom == '!' ||
       atom == '>' || atom == '<' ){
    GetAtom();
  }

  switch ( atom ){
  case '$' :
    GetAtom();
    err = gethex( &a );
    break;
  case '%':
    GetAtom();
    err = getbin( &a );
    break;
  case '*':
    GetAtom();
    a = Global.pc;
    err = EXPR_OK;
    break;
  case '@':
    GetAtom();
    a = Global.var;
    err = EXPR_OK;
    break;
  case '"':
    {
      char help[80];
      char *ptr = help;
      int i = 5;

      GetAtom();
      if ( !GetString(help, '"') ){
	Error(SYNTAX_ERR,"");
	return EXPR_ERROR;
      }
      while ( *ptr && --i){
	a = (a<<8)| (int)*ptr++;
      }

      err = EXPR_OK;
    }
    break;
  case '(':
    GetAtom();
    err = sum( &a );
    if ( !TestAtom(')') ){
      Error(EXPR_ERR,"");
      return EXPR_ERR;
    }
    break;
  case '0':
    if ( next_atom == 'x' || next_atom == 'X' ){
      GetAtom();
      GetAtom();
      err = gethex(&a);
      break;
    }
  default:
    if ( atom >= '0' && atom <= '9' ){
      err = getdec( &a );
    }  else {
      err = getlabel( &a );
    }
  }
  if ( err ) return err;

  switch ( save ){
  case '-' : a = -a; break;
  case '!' : a = !a; break;
  case '<' : a = a & 0xff; break;
  case '>' : a = (a >> 8) & 0xff; break;
  }
  *value = a;
  return EXPR_OK;
}



/*
  product ::= uni {("*"|"\"|"<<"|">>") uni}
*/
int product(long *value)
{
  int err;
  long a,b;

  err = uni( &a );

  if ( err ) return err;

  KillSpace();

  while( (atom == '*') ||
	 (atom == '/') ||
	 (atom == '&') ||
	 ((atom == '<') && next_atom == '<') ||
	 ((atom == '>') && next_atom == '>')){

    char save = atom;

    need_ch();

    if ( save == '<' || save == '>' ){
      if ( atom != save ){
	Error(EXPR_ERR,"");
	return EXPR_ERROR;
      }
      GetAtom();
    }

    err = uni( &b );

    if ( err ) return err;

    switch ( save ){
    case '*' : a *= b; break;
    case '/' : a /= b; break;
    case '&' : a &= b; break;
    case '<' : a <<=b; break;
    case '>' : a >>=b; break;
    }

    KillSpace();
  }

  *value = a;

  return EXPR_OK;
}
/*
  sum ::= product {("+"|"-"|"^"|"|") product}
*/
int sum(long *value)
{
  int err;
  long a,b;

  err = product( &a );

  if ( err ) return err;

  KillSpace();

  while ( (atom == '+') ||
	  (atom == '-') ||
	  (atom == '^') ||
	  (atom == '|') ){

    char save = atom;

    need_ch();

    err = product( &b );

    if ( err ) return err;

    switch ( save ){
    case '+' : a += b; break;
    case '-' : a -= b; break;
    case '^' : a ^= b; break;
    case '|' : a |= b; break;
    }
  }

  *value = a;
  return EXPR_OK;
}

int Expression( long * value)
{
  long v1,v2;
  int err;
  int flag = 0;
  *value = 0;
  ExprFailed = 0;

  if ( (err = sum( &v1 )) != EXPR_OK ) return err;

  KillSpace();

  switch ( atom ){
  case '=':
    GetAtom();
    if ( (err = sum( &v2 )) != EXPR_OK ) return err;
    *value = (v1 == v2);
    break;
  case '<' :
    GetAtom();
    flag = 0;
    if ( TestAtom('=') ){
      flag = 1;
    } else if ( TestAtom('>') ) {
      flag = 2;
    }
    if ( (err = sum( &v2 )) != EXPR_OK ) return err;
    switch( flag ){
    case 0 :
      *value = (v1 < v2);
      break;
    case 1 :
      *value = ( v1 <= v2);
      break;
    case 2 :
      *value = (v1 != v2);
      break;
    }
    break;
  case '>':
    flag = 0;
    GetAtom();
    if ( TestAtom('=') ){
      flag = 1;
    }
    if ( (err = sum( &v2 )) != EXPR_OK ) return err;
    if ( flag ){
      *value = (v1 >= v2);
    } else {
      *value = (v1 > v2 );
    }
    break;
  default:
    *value = v1;
  }

  if ( ExprFailed ){
    *value = 0;
    return EXPR_UNSOLVED;
  } else
    return EXPR_OK;
}

int NeedConst( long * value, const char * op)
{
  int err;

  if ( ( err = Expression(value) ) ){

    if ( err == EXPR_UNSOLVED ) return Error(NEEDCONST_ERR,op);

    return 1;
  }

  return 0;
}
