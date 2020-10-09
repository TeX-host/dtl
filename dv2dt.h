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
#include <stdlib.h>
#include <string.h>

#define OP_INFO_BOP \
    { 139, BOP, 11, "-4 -4 -4 -4 -4 -4 -4 -4 -4 -4 -4" }

#include "dtl.h"


#define PRINT_BCOM \
    if (group) fputc(BCOM_CHAR, dtl)
#define PRINT_ECOM \
    if (group) fputc(ECOM_CHAR, dtl)


/* function prototypes */

int open_dvi(char* dvi_file, FILE** dvi);
int open_dtl(char* dtl_file, FILE** dtl);
int dv2dt(FILE* dvi, FILE* dtl);

U4 xref_unsigned(int nBytes, FILE* dvi, FILE* dtl);
S4 xref_signed(int nBytes, FILE* dvi, FILE* dtl);

COUNT write_table(op_table table, int opcode, FILE* dvi, FILE* dtl);

COUNT set_seq(int opcode, FILE* dvi, FILE* dtl);
void set_pchar(int charcode, FILE* dtl);
void xfer_string(int nChars, FILE* dvi, FILE* dtl);

COUNT special(int nBytes, FILE* dvi, FILE* dtl);
COUNT fontdef(int nBytes, FILE* dvi, FILE* dtl);
COUNT preamble(FILE* dvi, FILE* dtl);
COUNT postamble(FILE* dvi, FILE* dtl);
COUNT postpost(FILE* dvi, FILE* dtl);

String program_name; /* name of dv2dt program */

#endif /* INC_DV2DT_H */