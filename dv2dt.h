#ifndef INC_DV2DT_H
/* dv2dt - convert DVI file to human-readable "DTL" format.

   This file is public domain.
   Originally written 1995, Geoffrey Tobin.
   The author has expressed the hope that any modification will retain enough
   content to remain useful. He would also appreciate being acknowledged as the
   original author in the documentation. This declaration added 2008/11/14 by
   Clea F. Rees with the permission of Geoffrey Tobin.

   - (ANSI C) version 0.6.0 - 17:54 GMT +11  Wed 8 March 1995
   - author:  Geoffrey Tobin    ecsgrt@luxor.latrobe.edu.au
   - patch:  Michal Tomczak-Jaegermann   ntomczak@vm.ucs.ualberta.ca
   - Reference:  "The DVI Driver Standard, Level 0",
                 by  The TUG DVI Driver Standards Committee.
                 Appendix A, "Device-Independent File Format".
*/
#define INC_DV2DT_H

/* unix version; read from stdin, write to stdout, by default. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dtl.h"

#define PRINT_BCOM \
    if (group) fprintf(dtl, "%s", BCOM)
#define PRINT_ECOM \
    if (group) fprintf(dtl, "%s", ECOM)

/*
  operation's:
     opcode,
     name,
     number of args,
     string of arguments.
*/
struct op_info_st {
    int code;
    char* name;
    int nargs;
    char* args;
};

typedef struct op_info_st op_info;

/*
  table's:
     name,
     first opcode,
     last opcode,
     pointer to opcode info.
*/
struct op_table_st {
    char* name;
    int first;
    int last;
    op_info* list;
};

typedef struct op_table_st op_table;

/* Table for opcodes 128 to 170 inclusive. */

op_info op_info_128_170[] = {
    {128, "s1", 1, "1"},
    {129, "s2", 1, "2"},
    {130, "s3", 1, "3"},
    {131, "s4", 1, "-4"},
    {132, "sr", 2, "-4 -4"},
    {133, "p1", 1, "1"},
    {134, "p2", 1, "2"},
    {135, "p3", 1, "3"},
    {136, "p4", 1, "-4"},
    {137, "pr", 2, "-4 -4"},
    {138, "nop", 0, ""},
    {139, "bop", 11, "-4 -4 -4 -4 -4 -4 -4 -4 -4 -4 -4"},
    {140, "eop", 0, ""},
    {141, "[", 0, ""},
    {142, "]", 0, ""},
    {143, "r1", 1, "-1"},
    {144, "r2", 1, "-2"},
    {145, "r3", 1, "-3"},
    {146, "r4", 1, "-4"},
    {147, "w0", 0, ""},
    {148, "w1", 1, "-1"},
    {149, "w2", 1, "-2"},
    {150, "w3", 1, "-3"},
    {151, "w4", 1, "-4"},
    {152, "x0", 0, ""},
    {153, "x1", 1, "-1"},
    {154, "x2", 1, "-2"},
    {155, "x3", 1, "-3"},
    {156, "x4", 1, "-4"},
    {157, "d1", 1, "-1"},
    {158, "d2", 1, "-2"},
    {159, "d3", 1, "-3"},
    {160, "d4", 1, "-4"},
    {161, "y0", 0, ""},
    {162, "y1", 1, "-1"},
    {163, "y2", 1, "-2"},
    {164, "y3", 1, "-3"},
    {165, "y4", 1, "-4"},
    {166, "z0", 0, ""},
    {167, "z1", 1, "-1"},
    {168, "z2", 1, "-2"},
    {169, "z3", 1, "-3"},
    {170, "z4", 1, "-4"}}; /* op_info  op_info_128_170 [] */

op_table op_128_170 = {"op_128_170", 128, 170, op_info_128_170};

/* Table for font with 1 to 4 bytes (opcodes 235 to 238) inclusive. */

op_info fnt_n[] = {{235, "f1", 1, "1"},
                   {236, "f2", 1, "2"},
                   {237, "f3", 1, "3"},
                   {238, "f4", 1, "-4"}}; /* op_info  fnt_n [] */

op_table fnt = {"f", 235, 238, fnt_n};

/* function prototypes */

int open_dvi ARGS((char* dvi_file, FILE** dvi));
int open_dtl ARGS((char* dtl_file, FILE** dtl));
int dv2dt ARGS((FILE * dvi, FILE* dtl));

COUNT wunsigned ARGS((int n, FILE* dvi, FILE* dtl));
COUNT wsigned ARGS((int n, FILE* dvi, FILE* dtl));
S4 rsigned ARGS((int n, FILE* dvi));
U4 runsigned ARGS((int n, FILE* dvi));

COUNT wtable ARGS((op_table table, int opcode, FILE* dvi, FILE* dtl));

COUNT setseq ARGS((int opcode, FILE* dvi, FILE* dtl));
Void setpchar ARGS((int charcode, FILE* dtl));
Void xferstring ARGS((int k, FILE* dvi, FILE* dtl));

COUNT special ARGS((FILE * dvi, FILE* dtl, int n));
COUNT fontdef ARGS((FILE * dvi, FILE* dtl, int n));
COUNT preamble ARGS((FILE * dvi, FILE* dtl));
COUNT postamble ARGS((FILE * dvi, FILE* dtl));
COUNT postpost ARGS((FILE * dvi, FILE* dtl));

String program; /* name of dv2dt program */

#endif /* INC_DV2DT_H */