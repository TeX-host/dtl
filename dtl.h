#ifndef INC_DTL_H
/* dtl.h
   
   This file is public domain.
   Originally written 1995, Geoffrey Tobin.
   The author has expressed the hope that any modification will retain enough content to remain useful. He would also appreciate being acknowledged as the original author in the documentation.
   This declaration added 2008/11/14 by Clea F. Rees with the permission of Geoffrey Tobin.

   - header for dv2dt.c and dt2dv.c, conversion programs
     for human-readable "DTL" <-> DVI.
   - (ANSI C) version 0.6.0 - 18:31 GMT +11  Wed 8 March 1995
   - author: Geoffrey Tobin    G.Tobin@ee.latrobe.edu.au
   - patch:  Michal Tomczak-Jaegermann   ntomczak@vm.ucs.ualberta.ca
   - Reference:  "The DVI Driver Standard, Level 0",
                 by  The TUG DVI Driver Standards Committee.
                 Appendix A, "Device-Independent File Format".
*/
#define INC_DTL_H
#include <inttypes.h>

/// variety of DTL produced
#define VARIETY     "sequences-6"

/// version of DTL programs
#define VERSION     "0.6.0"


/** types 
 */

/// types to store 4 byte signed and unsigned integers
typedef int32_t     S4;
typedef uint32_t    U4;
/// scanf and printf formats to read or write those
#define S4_FMT      "%"PRId32
#define U4_FMT      "%"PRIu32
/// 4 byte hexadecimal (checksum)
#define HEX_FMT     "%"PRIx32
/// 4 byte octal (checksum)
#define OCT_FMT     "%"PRIo32

/// type for byte count for DVI file 
/// COUNT must be large enough to hold a U4 (unsigned 4 byte) value 
typedef size_t      COUNT;
#define COUNT_FMT   "%zd"

/// size of a TeX and DVI word is 32 bits
typedef int32_t     word_t;
/// format for a DVI word
#define WORD_FMT    "%"PRId32

#define MAXSTRLEN   256
/// string of 8-bit characters for machine: keyboard, screen, memory 
typedef char        String[MAXSTRLEN+1];


/** global variable
 */

/// normally, debugging is off
int debug = 0;  

/// Is each DTL command parenthesised by a BCOM and an ECOM? 
/// by default, no grouping 
int group = 0;

/// name of this program
char* program_name;
void dtl_msg_start(char* level, const char* _file, int _ln, const char* _func) {
    fprintf(stderr, "%s", level);
    if (debug) {
        fprintf(stderr, "%s:%d: In function '%s': ", _file, _ln, _func);
    } else {
        fprintf(stderr, "%s: In function '%s': ", program_name, _func);
    }
}
#define _MSG_SATRT(level) dtl_msg_start(level, __FILE__, __LINE__, __func__)

#define MSG_SATRT   _MSG_SATRT("")
#define ERROR_SATRT _MSG_SATRT("[error] ")
#define WARN_SATRT  _MSG_SATRT("[warning] ")
#define INFO_SATRT  _MSG_SATRT("[info] ")
#define DEBUG_SATRT _MSG_SATRT("[debug] ")

/** signals of beginning and end of a command and its arguments 
 * these apply only if group is nonzero 
 */

#define BCOM "{"
#define ECOM "}"

#define BCOM_CHAR '{'
#define ECOM_CHAR '}'


/** beginning and end of a message string 
 */

#define  BMES  "'"
#define  EMES  BMES

#define  BMES_CHAR  '\''
#define  EMES_CHAR  BMES_CHAR


/** beginning and end of sequence of font characters 
 */

#define  BSEQ  "("
#define  ESEQ  ")"

#define  BSEQ_CHAR  '('
#define  ESEQ_CHAR  ')'


/** escape and quote characters
 */

#define  ESC_CHAR   '\\'
#define  QUOTE_CHAR '\"'


/** command names in DTL 
 */
enum DVICmd {
    SET1 = 128,
    SET2,
    SET3,
    SET4,
    SET_RULE,

    PUT1 = 133,
    PUT2,
    PUT3,
    PUT4,
    PUT_RULE,

    NOP = 138,
    BOP,
    EOP,
    PUSH,
    POP,

    RIGHT1 = 143,
    RIGHT2,
    RIGHT3,
    RIGHT4,

    W0 = 147,
    W1,
    W2,
    W3,
    W4,

    X0 = 152,
    X1,
    X2,
    X3,
    X4,

    DOWN1 = 157,
    DOWN2,
    DOWN3,
    DOWN4,

    Y0 = 161,
    Y1,
    Y2,
    Y3,
    Y4,

    Z0 = 166,
    Z1,
    Z2,
    Z3,
    Z4,

    FNT_NUM_0 = 171,
    /// [171, 234] fnt_num_N
    FNT_NUM_63 = 234,

    FONT1 = 235,
    FONT2,
    FONT3,
    FONT4,

    XXX1 = 239,
    XXX2,
    XXX3,
    XXX4,

    FNT_DEF1 = 243,
    FNT_DEF2,
    FNT_DEF3,
    FNT_DEF4,

    PRE = 247,
    POST,
    POSTPOST,

    UNDEFINED = 250,
}; /* enum DVICmd */

#define SETCHAR_STR     "\\"
#define SET_STR         "s"
#define SET1_STR        "s1"
#define SET2_STR        "s2"
#define SET3_STR        "s3"
#define SET4_STR        "s4"
#define SET_RULE_STR    "sr"
#define PUT_STR         "p"
#define PUT1_STR        "p1"
#define PUT2_STR        "p2"
#define PUT3_STR        "p3"
#define PUT4_STR        "p4"
#define PUT_RULE_STR    "pr"
#define NOP_STR         "nop"
#define BOP_STR         "bop"
#define EOP_STR         "eop"
#define PUSH_STR        "["
#define POP_STR         "]"
#define RIGHT_STR       "r"
#define RIGHT1_STR      "r1"
#define RIGHT2_STR      "r2"
#define RIGHT3_STR      "r3"
#define RIGHT4_STR      "r4"
#define W_STR           "w"
#define W0_STR          "w0"
#define W1_STR          "w1"
#define W2_STR          "w2"
#define W3_STR          "w3"
#define W4_STR          "w4"
#define X_STR           "x"
#define X0_STR          "x0"
#define X1_STR          "x1"
#define X2_STR          "x2"
#define X3_STR          "x3"
#define X4_STR          "x4"
#define DOWN_STR        "d"
#define DOWN1_STR       "d1"
#define DOWN2_STR       "d2"
#define DOWN3_STR       "d3"
#define DOWN4_STR       "d4"
#define Y_STR           "y"
#define Y0_STR          "y0"
#define Y1_STR          "y1"
#define Y2_STR          "y2"
#define Y3_STR          "y3"
#define Y4_STR          "y4"
#define Z_STR           "z"
#define Z0_STR          "z0"
#define Z1_STR          "z1"
#define Z2_STR          "z2"
#define Z3_STR          "z3"
#define Z4_STR          "z4"
#define FONT_STR        "f"
#define FONT1_STR       "f1"
#define FONT2_STR       "f2"
#define FONT3_STR       "f3"
#define FONT4_STR       "f4"
#define FONT_DEF_STR    "fd"
#define FONT_NUM_STR    "fn"
#define SPECIAL_STR     "special"
#define PRE_STR         "pre"
#define POST_STR        "post"
#define POSTPOST_STR    "post_post"
#define OPCODE_STR      "opcode"

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
    {SET1, SET1_STR, 1, "1"},
    {SET2, SET2_STR, 1, "2"},
    {SET3, SET3_STR, 1, "3"},
    {SET4, SET4_STR, 1, "-4"},
    {SET_RULE, SET_RULE_STR, 2, "-4 -4"},
    {PUT1, PUT1_STR, 1, "1"},
    {PUT2, PUT2_STR, 1, "2"},
    {PUT3, PUT3_STR, 1, "3"},
    {PUT4, PUT4_STR, 1, "-4"},
    {PUT_RULE, PUT_RULE_STR, 2, "-4 -4"},
    {NOP, NOP_STR, 0, ""},
    OP_INFO_BOP,
    {EOP, EOP_STR, 0, ""},
    {PUSH, PUSH_STR, 0, ""},
    {POP, POP_STR, 0, ""},
    {RIGHT1, RIGHT1_STR, 1, "-1"},
    {RIGHT2, RIGHT2_STR, 1, "-2"},
    {RIGHT3, RIGHT3_STR, 1, "-3"},
    {RIGHT4, RIGHT4_STR, 1, "-4"},
    {W0, W0_STR, 0, ""},
    {W1, W1_STR, 1, "-1"},
    {W2, W2_STR, 1, "-2"},
    {W3, W3_STR, 1, "-3"},
    {W4, W4_STR, 1, "-4"},
    {X0, X0_STR, 0, ""},
    {X1, X1_STR, 1, "-1"},
    {X2, X2_STR, 1, "-2"},
    {X3, X3_STR, 1, "-3"},
    {X4, X4_STR, 1, "-4"},
    {DOWN1, DOWN1_STR, 1, "-1"},
    {DOWN2, DOWN2_STR, 1, "-2"},
    {DOWN3, DOWN3_STR, 1, "-3"},
    {DOWN4, DOWN4_STR, 1, "-4"},
    {Y0, Y0_STR, 0, ""},
    {Y1, Y1_STR, 1, "-1"},
    {Y2, Y2_STR, 1, "-2"},
    {Y3, Y3_STR, 1, "-3"},
    {Y4, Y4_STR, 1, "-4"},
    {Z0, Z0_STR, 0, ""},
    {Z1, Z1_STR, 1, "-1"},
    {Z2, Z2_STR, 1, "-2"},
    {Z3, Z3_STR, 1, "-3"},
    {Z4, Z4_STR, 1, "-4"}
}; /* op_info  op_info_128_170 [] */

op_table op_128_170 = {"op_128_170", SET1, Z4, op_info_128_170};


/* Table for fnt1 to fnt4 (opcodes 235 to 238) inclusive. */
op_info fnt_n[] = {
    {FONT1, FONT1_STR, 1, "1"},
    {FONT2, FONT2_STR, 1, "2"},
    {FONT3, FONT3_STR, 1, "3"},
    {FONT4, FONT4_STR, 1, "-4"}
}; /* op_info  fnt_n [] */

op_table fnt = {FONT_STR, FONT1, FONT4, fnt_n};

#endif /* INC_DTL_H */