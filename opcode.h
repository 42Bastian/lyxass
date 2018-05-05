/*

  structure that describes an opcode

*/

struct opcode_s{
  char name[12];
  int (*func)(int);
  int misc;
};


int SearchOpcode(const struct opcode_s *, const char *);
int SearchOpcode2(const struct opcode_s *, const char *, int (**)(int ),int *);
