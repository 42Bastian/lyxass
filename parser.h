/*

  parser.h

*/
#ifndef _PARSER_H_
#define _PARSER_H_


extern int atom;
extern int next_atom;
extern int last_atom;
extern char * srcLinePtr;
extern char srcLine[1024];

int LoadSource(char *);

void SavePosition(void);
void RestorePosition(void);

int GetAtom(void);
int TestAtom(int x);
int TestAtomOR(int x, int y);
int GetLine();
void killLine(void);
int KillSpace();
int GetLabel(label_t *Label, int colon);
#define NO_COLON 0
#define W_COLON (!NO_COLON)

int GetCmd(void);
int GetComment(void);
int CheckForComma(void);
int GetString( char * ,char );
int GetFileName();
void InitParser();

/* expression.c */
int Expression( int32_t * value);
int NeedConst( int32_t * value, const char * op);
int Expression64( int64_t * value);
int NeedConst64( int64_t * value, const char * op);
int getdec32(int32_t *);

#endif /* _PARSER_H_ */
