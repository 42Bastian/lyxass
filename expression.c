/*

  expression.c


  Expression-parser



*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "my.h"
#include "label.h"
#include "error.h"
#include "global_vars.h"
#include "parser.h"

int Expression64(int64_t * value);
int sum(int64_t *value);

int ExprFailed;

/* returns:

   0 => expression ok => value is set
   1 => error in Expression
   2 => expression could not completly resolved, try later

*/


#define white(c) (((c) == ' ') || ((c) == '\t'))

// temp.

static label_t expr_label;

#define need_ch()\
    if ( ! GetAtom() ){\
      Error(EXPR_ERR,"");\
      return EXPR_ERR;\
    }

int getdec32(int32_t *value)
{
  int64_t dec = 0;
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

  if ( (dec < 0) || (i > 20) ){
    *value = 0;
    Error(TOOBIG_ERR,"");
    return EXPR_ERROR;
  }

  *value = dec;
  return EXPR_OK;
}


int getdec(int64_t *value)
{
  int64_t dec = 0;
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

  if ( (dec < 0) || (i > 20) ){
    *value = 0;
    Error(TOOBIG_ERR,"");
    return EXPR_ERROR;
  }

  *value = dec;
  return EXPR_OK;
}

int gethex(int64_t *value)
{
  int64_t hex = 0;
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

  if ( (i > 16) ){
    *value = 0;
    Error(TOOBIG_ERR,"");
    return EXPR_ERROR;
  }
  *value = hex;
  return EXPR_OK;
}

int getbin(int64_t *value)
{
  int64_t bin = 0;
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

  if ( i > 64 ){
    Error(TOOBIG_ERR,"");
    return EXPR_ERROR;
  }
  *value = bin;
  return EXPR_OK;
}

int getlabel(int64_t *value)
{
  int32_t lv = 0;
  int dummy;

  if ( GetLabel( &expr_label) ) return EXPR_ERROR;

  expr_label.type &= ~UNSURE;

  // printf("Type %02x\n",expr_label.type);

  if ( FindLabel( &expr_label, &lv ) ){
    if (expr_label.type & CODELABEL){
      ++Current.needsReloc;
//->      printf("Reloc: %s Line %4d %04x: %04x\n",file_list[Current.File].name,Current.Line,Global.pc,lv);
    }
    *value = (int64_t)lv;
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

int uni(int64_t *value)
{
  int err;
  char save;
  int64_t a = 0;


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
int product(int64_t *value)
{
  int err;
  int64_t a,b;

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
int sum(int64_t *value)
{
  int err;
  int64_t a,b;

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

int Expression64(int64_t * value)
{
  int64_t v1,v2;
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


int Expression(int32_t * pvalue)
{
  int err;
  int64_t value;

  *pvalue = 0;

  err = Expression64(&value);

  if ( err != EXPR_OK ){
    return err;
  }

  if ( (value > 0xffffffffLL) || (value < (int64_t)INT32_MIN) ){
    return Error(TOOBIG_ERR, 0);
  }

  *pvalue = (int32_t)value;

  return EXPR_OK;
}

int NeedConst64( int64_t * value, const char * op)
{

  int err;

  err = Expression64(value);

  if ( err == EXPR_UNSOLVED ) return Error(NEEDCONST_ERR,op);

  return err;
}

int NeedConst( int32_t * value, const char * op)
{
  int err;

  err = Expression(value);

  if ( err == EXPR_UNSOLVED ) {
    return Error(NEEDCONST_ERR,op);
  }

  return err;
}
