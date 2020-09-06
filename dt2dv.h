#ifndef INC_DT2DV_H
/* dt2dv - convert human-readable "DTL" file to DVI format
         - this is intended to invert dv2dt version 0.6.0

   This file is public domain.
   Originally written 1995, Geoffrey Tobin.
   The author has expressed the hope that any modification will retain enough
   content to remain useful. He would also appreciate being acknowledged as the
   original author in the documentation. This declaration added 2008/11/14 by
   Clea F. Rees with the permission of Geoffrey Tobin.

   - version 0.6.1 - 14:38 GMT +11  Thu 9 March 1995
   - Geoffrey Tobin    G.Tobin@ee.latrobe.edu.au
   - fixes:  Michal Tomczak-Jaegermann    ntomczak@vm.ucs.ualberta.ca
             Nelson H. F. Beebe    beebe@math.utah.edu
   - Reference:  "The DVI Driver Standard, Level 0",
                 by  The TUG DVI Driver Standards Committee.
                 Appendix A, "Device-Independent File Format".
*/
#define INC_DT2DV_H

/* unix version; read from stdin, write to stdout, by default. */

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dtl.h"

/* by default, read and write regular files */
int rd_stdin = 0;
int wr_stdout = 0;

/* maximum number of characters in a DTL input line */
#define MAXLINE 1024

/* input line */
typedef struct {
    COUNT num;   /* current line number */
    size_t max;  /* capacity of buf */
    S4 wrote;    /* number of characters written into buf */
    size_t read; /* position in buf of next character to read from buf */
    char* buf;   /* line buffer */
} Line;

char linebuf[MAXLINE + 1];

Line dtl_line = {0, 0, 0, MAXLINE, linebuf};

/* a DTL token either is:
     a quoted string (admitting an escape character),
     or BCOM (if that is a nonempty string),
     or ECOM (if that is a nonempty string),
     or a string _not_ including ECOM_CHAR or space.
*/

/* maximum expected length of a DTL token */
#define MAXTOKLEN 255
typedef char Token[MAXTOKLEN + 1];

typedef unsigned char Byte;
typedef char Boolean;

#define true 1
#define false 0

/* command prefixes */
typedef struct {
    Byte first_code;
    char* name;
    Boolean has_suffix;
    Byte first_suffix, last_suffix;
} CmdPrefix;

CmdPrefix cmd_prefixes[] = {
    {0, SETCHAR, true, 0, 127},   {128, SET, true, 1, 4},
    {132, SETRULE, false, 0, 0},  {133, PUT, true, 1, 4},
    {137, PUTRULE, false, 0, 0},  {138, NOP, false, 0, 0},
    {139, BOP, false, 0, 0},      {140, EOP, false, 0, 0},
    {141, PUSH, false, 0, 0},     {142, POP, false, 0, 0},
    {143, RIGHT, true, 1, 4},     {147, W, true, 0, 4},
    {152, X, true, 0, 4},         {157, DOWN, true, 1, 4},
    {161, Y, true, 0, 4},         {166, Z, true, 0, 4},
    {171, FONTNUM, true, 0, 63},  {235, FONT, true, 1, 4},
    {239, SPECIAL, true, 1, 4},   {243, FONTDEF, true, 1, 4},
    {247, PRE, false, 0, 0},      {248, POST, false, 0, 0},
    {249, POSTPOST, false, 0, 0}, {250, OPCODE, true, 250, 255}};
/* cmd_prefixes[] */

/* Number of DVI commands, including those officially undefined */
#define NCMDS 256

/* table of command name (string) pointers */
typedef char* CmdTable[NCMDS];

/* initially all command name pointers are NULL */
CmdTable cmd_table;

/* operation's opcode, name, number of args, string of arguments. */
typedef struct {
    int code;
    char* name;
    int nargs;
    char* args;
} op_info;

/* name of table, first opcode, last opcode, pointer to opcode info. */
typedef struct {
    char* name;
    int first;
    int last;
    op_info* list;
} op_table;

/* Table for opcodes 128 to 170 inclusive. */

op_info op_info_128_170[] = {
    {128, SET1, 1, "1"},
    {129, SET2, 1, "2"},
    {130, SET3, 1, "3"},
    {131, SET4, 1, "-4"},
    {132, SETRULE, 2, "-4 -4"},
    {133, PUT1, 1, "1"},
    {134, PUT2, 1, "2"},
    {135, PUT3, 1, "3"},
    {136, PUT4, 1, "-4"},
    {137, PUTRULE, 2, "-4 -4"},
    {138, NOP, 0, ""},
    /* bop:  not counting last argument, a signed address: */
    {139, BOP, 10, "-4 -4 -4 -4 -4 -4 -4 -4 -4 -4"},
    {140, EOP, 0, ""},
    {141, PUSH, 0, ""},
    {142, POP, 0, ""},
    {143, RIGHT1, 1, "-1"},
    {144, RIGHT2, 1, "-2"},
    {145, RIGHT3, 1, "-3"},
    {146, RIGHT4, 1, "-4"},
    {147, W0, 0, ""},
    {148, W1, 1, "-1"},
    {149, W2, 1, "-2"},
    {150, W3, 1, "-3"},
    {151, W4, 1, "-4"},
    {152, X0, 0, ""},
    {153, X1, 1, "-1"},
    {154, X2, 1, "-2"},
    {155, X3, 1, "-3"},
    {156, X4, 1, "-4"},
    {157, DOWN1, 1, "-1"},
    {158, DOWN2, 1, "-2"},
    {159, DOWN3, 1, "-3"},
    {160, DOWN4, 1, "-4"},
    {161, Y0, 0, ""},
    {162, Y1, 1, "-1"},
    {163, Y2, 1, "-2"},
    {164, Y3, 1, "-3"},
    {165, Y4, 1, "-4"},
    {166, Z0, 0, ""},
    {167, Z1, 1, "-1"},
    {168, Z2, 1, "-2"},
    {169, Z3, 1, "-3"},
    {170, Z4, 1, "-4"}};
/* op_info  op_info_128_170 [] */

op_table op_128_170 = {"op_128_170", 128, 170, op_info_128_170};

/* Table for fnt1 to fnt4 (opcodes 235 to 238) inclusive. */

op_info fnt_n[] = {{235, FONT1, 1, "1"},
                   {236, FONT2, 1, "2"},
                   {237, FONT3, 1, "3"},
                   {238, FONT4, 1, "-4"}};
/* op_info  fnt_n [] */

op_table fnt = {FONT, 235, 238, fnt_n};

/* Function prototypes */

Void mem_viol ARGS((int sig));
Void give_help(VOID);
int parse ARGS((char* s));
Void process ARGS((char* s));

Void no_op(VOID);
Void dtl_stdin(VOID);
Void dvi_stdout(VOID);

int open_dtl ARGS((char* dtl_file, FILE** pdtl));
int open_dvi ARGS((char* dvi_file, FILE** pdvi));

int dt2dv ARGS((FILE * dtl, FILE* dvi));

Void* gmalloc ARGS((long int size));

Void dinfo(VOID);
Void dexit ARGS((int n));

int cons_cmds ARGS((int nprefixes, CmdPrefix* prefix, CmdTable cmds));
Void free_cmds ARGS((CmdTable cmd_table));

int get_line ARGS((FILE * fp, Line* line, int max));
int read_line_char ARGS((FILE * fp, int* ch));
int read_char ARGS((FILE * fp, int* ch));
int unread_char(VOID);
int read_string_char ARGS((FILE * fp, int* ch));

COUNT read_variety ARGS((FILE * dtl));
COUNT read_token ARGS((FILE * dtl, char* token));
COUNT skip_space ARGS((FILE * fp, int* ch));
COUNT read_misc ARGS((FILE * fp, Token token));
COUNT read_mes ARGS((FILE * fp, char* token));

int find_command ARGS((char* command, int* opcode));
int xfer_args ARGS((FILE * dtl, FILE* dvi, int opcode));

int set_seq ARGS((FILE * dtl, FILE* dvi));

int check_byte ARGS((int byte));
int put_byte ARGS((int onebyte, FILE* dvi));

U4 xfer_hex ARGS((int n, FILE* dtl, FILE* dvi));
U4 xfer_oct ARGS((int n, FILE* dtl, FILE* dvi));
U4 xfer_unsigned ARGS((int n, FILE* dtl, FILE* dvi));
S4 xfer_signed ARGS((int n, FILE* dtl, FILE* dvi));

int check_bmes ARGS((FILE * dtl));
int check_emes ARGS((FILE * dtl));

Void init_Lstring ARGS((Lstring * lsp, long int n));
Void de_init_Lstring ARGS((Lstring * lsp));
Lstring* alloc_Lstring ARGS((long int n));
Void free_Lstring ARGS((Lstring * lstr));
Void ls_putb ARGS((int ch, Lstring* lstr));

S4 get_Lstring ARGS((FILE * dtl, Lstring* lstr));
Void put_Lstring ARGS((const Lstring* lstr, FILE* dvi));
U4 xfer_len_string ARGS((int n, FILE* dtl, FILE* dvi));

U4 get_unsigned ARGS((FILE * dtl));
S4 get_signed ARGS((FILE * dtl));

int put_unsigned ARGS((int n, U4 unum, FILE* dvi));
int put_signed ARGS((int n, S4 snum, FILE* dvi));

S4 xfer_bop_address ARGS((FILE * dtl, FILE* dvi));
S4 xfer_postamble_address ARGS((FILE * dtl, FILE* dvi));

int put_table ARGS((op_table table, int opcode, FILE* dtl, FILE* dvi));

U4 special ARGS((FILE * dtl, FILE* dvi, int n));
int fontdef ARGS((FILE * dtl, FILE* dvi, int n));

U4 preamble ARGS((FILE * dtl, FILE* dvi));
int postamble ARGS((FILE * dtl, FILE* dvi));
int post_post ARGS((FILE * dtl, FILE* dvi));

typedef struct {
    char* keyword;      /* command line option keyword */
    int* p_var;         /* pointer to option variable */
    char* desc;         /* description of keyword and value */
    Void (*p_fn)(VOID); /* pointer to function called when option is set */
} Options;

Options opts[] = {
    {"-debug", &debug, "detailed debugging", no_op},
    {"-group", &group, "each DTL command is in parentheses", no_op},
    {"-si", &rd_stdin, "read all DTL commands from standard input", dtl_stdin},
    {"-so", &wr_stdout, "write all DVI commands to standard output",
     dvi_stdout},
    {NULL, NULL, NULL, NULL}};
/* opts[] */

char* progname = ""; /* intended for name of this program */
int nfile = 0;       /* number of filename arguments on the command line */

#define PRINT_PROGNAME fprintf(stderr, "%s ", progname)

/* Harbison & Steele (1991) warn that some C implementations */
/* of free() do not treat NULL pointers correctly. */
#define gfree(p)        \
    {                   \
        if (p) free(p); \
    }

FILE* dtl_fp = NULL;
FILE* dvi_fp = NULL;

char* dtl_filename = "";
char* dvi_filename = "";

#endif /* INC_DT2DV_H */