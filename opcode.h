/*

  structure that describes an opcode

*/
#ifndef _OPCODE_H_
#define _OPCODE_H_
struct opcode_s{
  char name[12];
  int (*func)(int);
  int misc;
};

int SearchOpcode(const struct opcode_s *, const char *);
int SearchOpcode2(const struct opcode_s *, const char *, int (**)(int ),int *);

#endif /* _OPCODE_H_ */
