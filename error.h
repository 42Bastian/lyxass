#ifdef MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

int Error(int , const char *);
void Warning(const char *);

//EXTERN int err;
//EXTERN int err_num;

#define INCLUDE_ERR	1
#define FILE_ERR       -2
#define LABEL_ERR       3
#define OP_ERR		4
#define MLABEL_ERR      5
#define NOMEM_ERR      -6
#define REDEF_ERR       7
#define LMREDEF_ERR     8
#define LREDEF_ERR      9
#define MREDEF_ERR     10
#define TOOMANY_ERR    11
#define LMTOOMANY_ERR  12
#define LTOOMANY_ERR   13
#define MTOOMANY_ERR   14
#define UNKNOWN_ERR    15
#define TOOBIG_ERR     16
#define EXPR_ERR       17
#define SYNTAX_ERR     18
#define NEEDCONST_ERR  19
#define MLABEL2_ERR    20
#define CODEMEM_ERR    21
#define DISTANCE_ERR   22
#define NOREFMEM_ERR  -23
#define GARBAGE_ERR    24
#define MACRONESTED_ERR 25
#define BYTE_ERR       26
#define WORD_ERR       27
#define TYPE_ERR       28
#define UNSOLVED_ERR   29
#define TOOMANYIF_ERR  30
#define ENDIF1_ERR     31
#define FAIL_ERR       32
#define SWITCH_ERR     33
#define CASE_ERR       34
#define ENDS_ERR       35
#define REPT1_ERR      36
#define REPT2_ERR      37
#define DEFAULT_ERR    38
#define ISYMS_ERR     -39
#define CMD_ERR       -40
#define WRITE_ERR     -41
#define LOAD_ERR      -42
// jaguar-errors
#define IMM_ERR        43
#define REG_ERR        44
#define REG1_ERR        45
#define MISC_ERR   46
