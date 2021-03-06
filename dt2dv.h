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

/// bop:  not counting last argument, a signed address:
#define OP_INFO_BOP \
    { BOP, BOP_STR, 10, "-4 -4 -4 -4 -4 -4 -4 -4 -4 -4" }

#include "dtl.h"


/** Set command-line options.
 *
 */

/// command line options.
typedef struct _Options {
    const char* keyword; /* command line option keyword */
    int* p_var;          /* pointer to option variable */
    const char* desc;    /* description of keyword and value */
    void (*p_fn)(void);  /* pointer to function called when option is set */
} Options;

/* by default, read and write regular files */
int rd_stdin = 0;
int wr_stdout = 0;

void no_op(void);
void dtl_stdin(void);
void dvi_stdout(void);

Options opts[] = {
    {"-debug", &debug, "detailed debugging", no_op},
    {"-group", &group, "each DTL command is in parentheses", no_op},
    {"-si", &rd_stdin, "read all DTL commands from standard input", dtl_stdin},
    {"-so", &wr_stdout, "write all DVI commands to standard output",
     dvi_stdout},
    {NULL, NULL, NULL, NULL}
}; /* opts[] */


/* Size typically used in this program for LString variables */
#define LSTR_SIZE 1024

/* string s of length l and maximum length m */
typedef struct _LString {
    size_t l; ///< string length.
    size_t m; ///< string maximum length.
    char* s;  ///< string.
} LString;
typedef LString* LStringPtr;


typedef enum _CharStatus {
    CHAR_EOS = -1, ///< end of LString.
    CHAR_FAIL,
    CHAR_OK,
} CharStatus;


/* maximum number of characters in a DTL input line */
#define MAX_LINE 1024

/* input line */
typedef struct _Line {
    COUNT num;   /* current line number */
    size_t max;  /* capacity of buf */
    S4 wrote;    /* number of characters written into buf */
    size_t read; /* position in buf of next character to read from buf */
    char* buf;   /* line buffer */
} Line;

char linebuf[MAX_LINE + 1];
Line dtl_line = {0, 0, 0, MAX_LINE, linebuf};


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
    {0, SETCHAR_STR, true, 0, 127},   {128, SET_STR, true, 1, 4},
    {132, SET_RULE_STR, false, 0, 0}, {133, PUT_STR, true, 1, 4},
    {137, PUT_RULE_STR, false, 0, 0}, {138, NOP_STR, false, 0, 0},
    {139, BOP_STR, false, 0, 0},      {140, EOP_STR, false, 0, 0},
    {141, PUSH_STR, false, 0, 0},     {142, POP_STR, false, 0, 0},
    {143, RIGHT_STR, true, 1, 4},     {147, W_STR, true, 0, 4},
    {152, X_STR, true, 0, 4},         {157, DOWN_STR, true, 1, 4},
    {161, Y_STR, true, 0, 4},         {166, Z_STR, true, 0, 4},
    {171, FONT_NUM_STR, true, 0, 63}, {235, FONT_STR, true, 1, 4},
    {239, SPECIAL_STR, true, 1, 4},   {243, FONT_DEF_STR, true, 1, 4},
    {247, PRE_STR, false, 0, 0},      {248, POST_STR, false, 0, 0},
    {249, POSTPOST_STR, false, 0, 0}, {250, OPCODE_STR, true, 250, 255}};
/* cmd_prefixes[] */

/* Number of DVI commands, including those officially undefined */
#define NCMDS 256

/* table of command name (string) pointers */
typedef char* CmdTable[NCMDS];

/* initially all command name pointers are NULL */
CmdTable cmd_table;


int nfile = 0;       /* number of filename arguments on the command line */

FILE* dtl_fp = NULL;
FILE* dvi_fp = NULL;

char* dtl_filename = "";
char* dvi_filename = "";

COUNT dtl_read = 0;            /* bytes read from dtl file */
COUNT dvi_written = 0;         /* bytes written to dvi file */
word_t last_bop_address = -1;  /* byte address of last bop; first bop uses -1 */
word_t postamble_address = -1; /* byte address of postamble */
COUNT ncom = 0; /* commands successfully read and interpreted from dtl file */
COUNT com_read = 0; /* bytes read in current (command and arguments), */
                    /* since and including the opening BCOM_CHAR, if any */


/* Function prototypes */

void mem_viol(int sig);
void give_help(void);
int parse(char* s);
void process(char* s);

int open_dtl(char* dtl_file, FILE** pdtl);
int open_dvi(char* dvi_file, FILE** pdvi);

int dt2dv(FILE* dtl, FILE* dvi);

void* gmalloc(size_t size);

void dinfo(void);
void dexit(int n);

int cons_cmds(int nprefixes, CmdPrefix prefix[], CmdTable cmds);
void free_cmds(CmdTable cmd_table);

int get_line(FILE* fp, Line* line, int max);
int read_line_char(FILE* fp, int* ch);
int read_char(FILE* fp, int* ch);
int unread_char(void);
CharStatus read_string_char(FILE* fp, int* ch);

COUNT read_variety(FILE* dtl);
COUNT read_token(FILE* dtl, char* token);
COUNT skip_space(FILE* fp, int* ch);
COUNT read_misc(FILE* fp, Token token);
COUNT read_mes(FILE* fp, char* token);

int find_command(char* command, int* opcode);
int xfer_args(FILE* dtl, FILE* dvi, int opcode);

int set_seq(FILE* dtl, FILE* dvi);

int check_byte(int byte);
int put_byte(int onebyte, FILE* dvi);

U4 xfer_hex(int n, FILE* dtl, FILE* dvi);
U4 xfer_oct(int n, FILE* dtl, FILE* dvi);
U4 xfer_unsigned(int n, FILE* dtl, FILE* dvi);
S4 xfer_signed(int n, FILE* dtl, FILE* dvi);

int check_bmes(FILE* dtl);
int check_emes(FILE* dtl);

void init_lstr(LStringPtr lsp, size_t n);
void clear_lstr(LStringPtr lsp);
LStringPtr alloc_lstr(size_t n);
void free_lstr(LStringPtr lsp);

void putch_lstr(int ch, LStringPtr lsp);
size_t get_lstr(FILE* dtl, LStringPtr lsp);
void put_lstr(LStringPtr lsp, FILE* dvi);
U4 xfer_len_string(int n, FILE* dtl, FILE* dvi);

U4 get_unsigned(FILE* dtl);
S4 get_signed(FILE* dtl);

int put_unsigned(int n, U4 unum, FILE* dvi);
int put_signed(int n, S4 snum, FILE* dvi);

S4 xfer_bop_address(FILE* dtl, FILE* dvi);
S4 xfer_postamble_address(FILE* dtl, FILE* dvi);

int put_table(op_table table, int opcode, FILE* dtl, FILE* dvi);

U4 special(FILE* dtl, FILE* dvi, int n);
int fontdef(FILE* dtl, FILE* dvi, int n);

U4 preamble(FILE* dtl, FILE* dvi);
int postamble(FILE* dtl, FILE* dvi);
int post_post(FILE* dtl, FILE* dvi);

#endif /* INC_DT2DV_H */