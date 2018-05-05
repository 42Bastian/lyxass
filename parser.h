/*

  parser.h

*/



extern int atom;
extern int next_atom;
extern char * srcLinePtr;
extern char srcLine[256];

int LoadSource(char *);

void SavePosition(void);
void RestorePosition(void);

int GetAtom(void);
int TestAtom(int x);
int TestAtomOR(int x, int y);
int GetLine();
void killLine(void);
int KillSpace();
int GetLabel(LABEL *Label);
int GetCmd(void);
int GetComment(void);
int CheckForComma(void);
int GetString( char * ,char );
int GetFileName();

void InitParser();

