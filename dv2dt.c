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
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <ctype.h>  // isprint
#include "dv2dt.h"


int main(int argc, char* argv[]) {
    FILE* dvi = stdin;
    FILE* dtl = stdout;

    program_name = argv[0];

    if (argc > 1) open_dvi(argv[1], &dvi);
    if (argc > 2) open_dtl(argv[2], &dtl);

    return dv2dt(dvi, dtl);
} /* end main */


/** Open DVI file for input.
 * 
 *  @param[in]      dvi_fname
 *  @param[inout]   pdvi
 */
int open_dvi(char* dvi_fname, FILE** pdvi) {
    if (pdvi == NULL) {
        ERROR_SATRT;
        fprintf(stderr, "address of dvi variable is NULL.\n");
        exit(EXIT_FAILURE);
    }

    *pdvi = fopen(dvi_fname, "rb");

    if (*pdvi == NULL) {
        ERROR_SATRT;
        fprintf(stderr, "Cannot open \"%s\" for binary reading.\n",
                dvi_fname);
        exit(EXIT_FAILURE);
    }

    return 1; /* OK */
} /* open_dvi */

/** Open DTL file for output.
 *
 *  @param[in]      dtl_fname
 *  @param[inout]   pdtl
 */
int open_dtl(char* dtl_fname, FILE** pdtl) {
    if (pdtl == NULL) {
        ERROR_SATRT;
        fprintf(stderr, "address of dtl variable is NULL.\n");
        exit(EXIT_FAILURE);
    }

    *pdtl = fopen(dtl_fname, "w");

    if (*pdtl == NULL) {
        ERROR_SATRT;
        fprintf(stderr, "Cannot open \"%s\" for text writing.\n",
                dtl_fname);
        exit(EXIT_FAILURE);
    }

    return 1; /* OK */
} /* open_dtl */

int dv2dt(FILE* dvi, FILE* dtl) {
    int opcode;
    COUNT count; /* intended to count bytes to DVI file; as yet unused. */

    PRINT_BCOM;
    fputs("variety ", dtl);
    /* fputc(BMES_CHAR, dtl); */
    fputs(VARIETY, dtl);
    /* fputc(EMES_CHAR, dtl); */
    PRINT_ECOM;
    fputc('\n', dtl);

    /* start counting DVI bytes */
    count = 0;
    while ((opcode = fgetc(dvi)) != EOF) {
        PRINT_BCOM; /* start of command and parameters */
        if (opcode < 0 || opcode > 255) {
            count += 1;
            ERROR_SATRT;
            fprintf(stderr, "Non-byte from \"fgetc()\"!\n");
            exit(EXIT_FAILURE);
        } else if (opcode <= 127) {
            /* setchar commands */
            /* count += 1; */
            /* fprintf (dtl, "%s%d", SETCHAR, opcode); */
            count += set_seq(opcode, dvi, dtl);
        } else if (opcode >= 128 && opcode <= 170) {
            count += write_table(op_128_170, opcode, dvi, dtl);
        } else if (opcode >= 171 && opcode <= 234) {
            count += 1;
            fprintf(dtl, "%s%d", FONTNUM, opcode - 171);
        } else if (opcode >= 235 && opcode <= 238) {
            count += write_table(fnt, opcode, dvi, dtl);
        } else if (opcode >= 239 && opcode <= 242) {
            count += special(opcode - 238, dvi, dtl);
        } else if (opcode >= 243 && opcode <= 246) {
            count += fontdef(opcode - 242, dvi, dtl);
        } else if (opcode == 247) {
            count += preamble(dvi, dtl);
        } else if (opcode == 248) {
            count += postamble(dvi, dtl);
        } else if (opcode == 249) {
            count += postpost(dvi, dtl);
        } else if (opcode >= 250 && opcode <= 255) {
            count += 1;
            fprintf(dtl, "opcode%d", opcode);
        } else {
            count += 1;
            ERROR_SATRT;
            fprintf(stderr, "unknown byte.\n");
            exit(EXIT_FAILURE);
        }
        PRINT_ECOM; /* end of command and parameters */
        fputc('\n', dtl);
        if (fflush(dtl) == EOF) {
            ERROR_SATRT;
            fprintf(stderr, "fflush on dtl file gave write error!\n");
            exit(EXIT_FAILURE);
        }
    } /* end while */

    return EXIT_SUCCESS;
} /* dv2dt */


/** read 1 <= n <= 4 bytes for an unsigned integer from dvi file
 * DVI format uses Big-endian storage of numbers.
 *
 *  @param[in] n number of bytes to read
 *  @param[in] dvi file
 *  @return unsign int
 */
U4 read_unsigned(int nBytes, FILE* dvi) {
    U4 integer = 0;
    int ibyte = 0;

    if (nBytes < 1 || nBytes > 4) {
        ERROR_SATRT;
        fprintf(stderr,
                "read_unsigned() asked for %d bytes.  Must be 1 to 4.\n",
                nBytes);
        exit(EXIT_FAILURE);
    }

    /* Following calculation works iff storage is big-endian. */
    for (int i = 0; i < nBytes; i++) {
        integer *= 256;
        ibyte = fgetc(dvi);
        integer += ibyte;
    }

    return integer;
} /* end read_unsigned */

/** transfer n bytes as an unsign int from dvi to dtl.
 *
 *  @param[in]  nBytes  number of bytes to be transfered
 *  @param[in]  dvi     input DVI file
 *  @param[out] dtl     output DTL file
 */
U4 xref_unsigned(int nBytes, FILE* dvi, FILE* dtl) {
    U4 unum;

    fputc(' ', dtl);
    unum = read_unsigned(nBytes, dvi);
    fprintf(dtl, U4_FMT, unum);

    return unum;
} /* end xref_unsigned */

/** read 1 <= n <= 4 bytes for a signed integer from dvi file
 *  DVI format uses Big-endian storage of numbers.
 *
 *  @param[in] n number of bytes to read
 *  @param[in] dvi file
 *  @return sign int
 */
S4 read_signed(int nBytes, FILE* dvi) {
    S4 integer = 0;
    int ibyte = 0;

    if (nBytes < 1 || nBytes > 4) {
        ERROR_SATRT;
        fprintf(stderr, 
                "read_signed() asked for %d bytes.  Must be 1 to 4.\n",
                nBytes);
        exit(EXIT_FAILURE);
    }

    /* Following calculation works iff storage is big-endian. */
    for (int i = 0; i < nBytes; i++) {
        integer *= 256;
        ibyte = fgetc(dvi);
        /* Big-endian implies sign byte is first byte. */
        if (i == 0 && ibyte >= 128) {
            ibyte -= 256;
        }
        integer += ibyte;
    }

    return integer;
} /* end read_signed */

/** transfer n bytes as a sign int from dvi to dtl.
 *
 *  @param[in]  nBytes  number of bytes to be transfered
 *  @param[in]  dvi     input DVI file
 *  @param[out] dtl     output DTL file
 */
S4 xref_signed(int nBytes, FILE* dvi, FILE* dtl) {
    S4 snum;

    fputc(' ', dtl);
    snum = read_signed(nBytes, dvi);
    fprintf(dtl, S4_FMT, snum);

    return snum;
} /* end xref_signed */

/** Write command with given opcode in given table.
 *
 *  @param[in]  table
 *  @param[in]  opcode
 *  @param[in]  dvi
 *  @param[out] dtl
 *  @return bytes_count  number of DVI bytes in this command
 */
COUNT write_table(op_table table, int opcode, FILE* dvi, FILE* dtl) {
    op_info op; /* pointer into table of operations and arguments */
    COUNT bytes_count = 0; /* number of bytes in arguments of this opcode */
    String args; /* arguments string */
    int arg_pos; /* position in args */

    /* Defensive programming. */
    if (opcode < table.first || opcode > table.last) {
        ERROR_SATRT;
        fprintf(stderr, "opcode %d is outside table %s [ %d to %d ] !\n",
                opcode, table.name, table.first, table.last);
        exit(EXIT_FAILURE);
    }

    op = table.list[opcode - table.first];

    /* Further defensive programming. */
    if (op.code != opcode) {
        fprintf(stderr, "internal table %s wrong!\n", 
                table.name);
        exit(EXIT_FAILURE);
    }

    bytes_count = 1;
    fputs(op.name, dtl);

    /* NB:  sscanf does an ungetc, */
    /*      so args must be writable. */

    strncpy(args, op.args, MAXSTRLEN);

    arg_pos = 0;
    for (int i = 0; i < op.nargs; i++) {
        int arg_type; /* sign and number of bytes in current argument */
        int n_conv;   /* number of successful conversions from args */
        int n_read;   /* number of bytes read from args */

        n_conv = sscanf(args + arg_pos, "%d%n", &arg_type, &n_read);

        /* internal consistency checks */
        if (n_conv != 1 || n_read <= 0) {
            ERROR_SATRT;
            fprintf(stderr, "internal read of table %s failed!\n",
                    table.name);
            exit(EXIT_FAILURE);
        }

        arg_pos += n_read;
        bytes_count += (arg_type < 0 ? xref_signed(-arg_type, dvi, dtl)
                                     : xref_unsigned(arg_type, dvi, dtl));
    } /* end for */

    return bytes_count;
} /* write_table */

/** Write a sequence of setchar commands.
 *
 *  @param[in]  opcode
 *  @param[in]  dvi
 *  @param[out] dtl
 *  @return count of DVI bytes interpreted into DTL.
 */
COUNT set_seq(int opcode, FILE* dvi, FILE* dtl) {
    int char_code = opcode; /* fortuitous */
    int char_count = 0;

    if (!isprint(char_code)) {
        fprintf(dtl, "%s%02X", SETCHAR, opcode);
        char_count++;
        return char_count;
    }

    /*  @assert( isprint(char_code) )  */

    /* start of sequence of font characters */
    fputc(BSEQ_CHAR, dtl);

    /* first character */
    set_pchar(char_code, dtl);
    char_count++;

    /* subsequent characters */
    while ((opcode = fgetc(dvi)) != EOF) {
        if (opcode < 0 || opcode > 127) {
            break; /* not a setchar command, so sequence has ended */
        }

        char_code = opcode;      /* fortuitous */
        if (!isprint(char_code)) { /* not printable ascii */
            /* end of font character sequence, as for other commands */
            break;
        } else { /* printable ASCII */
            set_pchar(char_code, dtl);
            char_count++;
        }
    } /* end for loop */

    /* prepare to reread opcode of next DVI command */
    if (ungetc(opcode, dvi) == EOF) {
        fprintf(stderr, "set_seq:  cannot push back a byte\n");
        exit(EXIT_FAILURE);
    }

    /* end of sequence of font characters */
    fputc(ESEQ_CHAR, dtl);

    return char_count;
} /* set_seq */

/** set printable character.
 * 
 *  @param[in]  charcode
 *  @param[out] dtl
 *  @return void
 */
void set_pchar(int charcode, FILE* dtl) {
    switch (charcode) {
        case ESC_CHAR:
            fputc(ESC_CHAR, dtl);
            fputc(ESC_CHAR, dtl);
            break;
        case QUOTE_CHAR:
            fputc(ESC_CHAR, dtl);
            fputc(QUOTE_CHAR, dtl);
            break;
        case BSEQ_CHAR:
            fputc(ESC_CHAR, dtl);
            fputc(BSEQ_CHAR, dtl);
            break;
        case ESEQ_CHAR:
            fputc(ESC_CHAR, dtl);
            fputc(ESEQ_CHAR, dtl);
            break;
        default:
            fputc(charcode, dtl);
            break;
    }
} /* set_pchar */

/** Copy string of n characters from DVI file to DTL file.
 *
 *  @param[in]  nChars
 *  @param[in]  dvi
 *  @param[out] dtl
 */
void xfer_string(int nChars, FILE* dvi, FILE* dtl) {
    fputc(' ', dtl);
    fputc('\'', dtl);

    for (int i = 0; i < nChars; i++) {
        int ch = fgetc(dvi);

        if (ch == ESC_CHAR || ch == EMES_CHAR) {
            fputc(ESC_CHAR, dtl);
        }
        fputc(ch, dtl);
    }

    fputc('\'', dtl);
} /* xfer_string */

/** read special 1 .. 4 from dvi and write in dtl.
 *
 *  @param[in]  nBytes
 *  @param[in]  dvi
 *  @param[out] dtl
 *  @return number of DVI bytes interpreted into DTL.
 */
COUNT special(int nBytes, FILE* dvi, FILE* dtl) {
    U4 k;

    if (nBytes < 1 || nBytes > 4) {
        ERROR_SATRT;
        fprintf(stderr, "special %d, range is 1 to 4.\n", 
                nBytes);
        exit(EXIT_FAILURE);
    }

    fprintf(dtl, "%s%d", SPECIAL, nBytes);
    k = xref_unsigned(nBytes, dvi, dtl); /* k[n] = length of special string */
    xfer_string(k, dvi, dtl);            /* x[k] = special string */

    return (1 + nBytes + k);
} /* end special */

/** read fontdef 1 .. 4 from dvi and write in dtl
 *
 *  @param[in]  nBytes
 *  @param[in]  dvi
 *  @param[out] dtl
 *  @return number of DVI bytes interpreted into DTL.
 */
COUNT fontdef(int nBytes, FILE* dvi, FILE* dtl) {
    U4 c, a, l;

    if (nBytes < 1 || nBytes > 4) {
        ERROR_SATRT;
        fprintf(stderr, "font def %d, range is 1 to 4.\n", 
                nBytes);
        exit(EXIT_FAILURE);
    }

    fprintf(dtl, "%s%d", FONTDEF, nBytes);

    /* k[n] = font number */
    if (nBytes == 4) {
        xref_signed(nBytes, dvi, dtl);
    } else {
        xref_unsigned(nBytes, dvi, dtl);
    }

    /* c[4] = checksum */
    fputc(' ', dtl);
    c = read_unsigned(4, dvi);

#ifdef HEX_CHECKSUM
    fprintf(dtl, HEX_FMT, c);
#else /* NOT HEX_CHECKSUM */
    /* write in octal, to allow quick comparison with tftopl's output */
    fprintf(dtl, OCT_FMT, c);
#endif

    xref_unsigned(4, dvi, dtl);     /*   s[4] = scale factor */
    xref_unsigned(4, dvi, dtl);     /*   d[4] = design size */
    a = xref_unsigned(1, dvi, dtl); /*   a[1] = length of area (directory) name */
    l = xref_unsigned(1, dvi, dtl); /*   l[1] = length of font name */
    xfer_string(a+l, dvi, dtl);     /* n[a+l] = font pathname string 
                                                => area (directory) + font */

    return (1 + nBytes + 4 + 4 + 4 + 1 + 1 + a + l);
} /* end fontdef */

/** read preamble from dvi and write in dtl
 *
 *  @return number of DVI bytes interpreted into DTL
 */
COUNT preamble(FILE* dvi, FILE* dtl) {
    U4 k;

    fputs("pre", dtl);
    xref_unsigned(1, dvi, dtl);     /*   i[1] = DVI format identification   */
    xref_unsigned(4, dvi, dtl);     /* num[4] = numerator of DVI unit       */
    xref_unsigned(4, dvi, dtl);     /* den[4] = denominator of DVI unit     */
    xref_unsigned(4, dvi, dtl);     /* mag[4] = 1000 x magnification        */
    k = xref_unsigned(1, dvi, dtl); /*   k[1] = length of comment           */
    xfer_string(k, dvi, dtl);       /*   x[k] = comment string              */

    return (1 + 1 + 4 + 4 + 4 + 1 + k);
} /* end preamble */

/** read postamble from dvi and write in dtl.
 *
 *  @return number of bytes
 */
COUNT postamble(FILE* dvi, FILE* dtl) {
    fputs("post", dtl);
    xref_unsigned(4, dvi, dtl); /*   p[4] = pointer to final bop            */
    xref_unsigned(4, dvi, dtl); /* num[4] = numerator of DVI unit           */
    xref_unsigned(4, dvi, dtl); /* den[4] = denominator of DVI unit         */ 
    xref_unsigned(4, dvi, dtl); /* mag[4] = 1000 x magnification            */
    xref_unsigned(4, dvi, dtl); /*   l[4] = height + depth of tallest page  */
    xref_unsigned(4, dvi, dtl); /*   u[4] = width of widest page            */
    xref_unsigned(2, dvi, dtl); /*   s[2] = maximum stack depth             */
    xref_unsigned(2, dvi, dtl); /*   t[2] = total number of pages 
                                                (bop commands)              */

    return (1 + 4 + 4 + 4 + 4 + 4 + 4 + 2 + 2); // 29
} /* end postamble */

/** read post_post from dvi and write in dtl.
 * 
 *  @return  number of bytes
 */
COUNT postpost(FILE* dvi, FILE* dtl) {
    int b223; /* hope this is 8-bit clean */
    int n223; /* number of "223" bytes in final padding */

    fputs("post_post", dtl);
    xref_unsigned(4, dvi, dtl); /* q[4] = pointer to post command */
    xref_unsigned(1, dvi, dtl); /* i[1] = DVI identification byte */

    /* final padding by "223" bytes */
    /* hope this way of obtaining b223 is 8-bit clean */
    for (n223 = 0; (b223 = fgetc(dvi)) == 223; n223++) {
        fputc(' ', dtl);
        fputc(223, dtl);
    }
    if (n223 < 4) {
        ERROR_SATRT;
        fprintf(stderr, "bad post_post:  fewer than four \"223\" bytes.\n");
        exit(EXIT_FAILURE);
    }
    if (b223 != EOF) {
        ERROR_SATRT;
        fprintf(stderr, "bad post_post:  doesn't end with a \"223\".\n");
        exit(EXIT_FAILURE);
    }

    return (1 + 4 + 1 + n223);
} /* end postpost */

/* end of "dv2dt.c" */