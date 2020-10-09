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

    /* Watch out:  C's standard library's string functions are dicey */
    strncpy(program_name, argv[0], MAXSTRLEN);

    if (argc > 1) open_dvi(argv[1], &dvi);
    if (argc > 2) open_dtl(argv[2], &dtl);

    return dv2dt(dvi, dtl);
} /* end main */


/** Open DVI file for input.
 * 
 *  @param[in]      dvi_file
 *  @param[inout]   pdvi
 */
int open_dvi(char* dvi_file, FILE** pdvi) {
    if (pdvi == NULL) {
        fprintf(stderr, "%s:  address of dvi variable is NULL.\n",
                program_name);
        exit(EXIT_FAILURE);
    }

    *pdvi = fopen(dvi_file, "rb");

    if (*pdvi == NULL) {
        fprintf(stderr, "%s:  Cannot open \"%s\" for binary reading.\n",
                program_name, dvi_file);
        exit(EXIT_FAILURE);
    }

    return 1; /* OK */
} /* open_dvi */

/** Open DTL file for output.
 *
 *  @param[in]      dtl_file
 *  @param[inout]   pdtl
 */
int open_dtl(char* dtl_file, FILE** pdtl) {
    if (pdtl == NULL) {
        fprintf(stderr, "%s:  address of dtl variable is NULL.\n",
                program_name);
        exit(EXIT_FAILURE);
    }

    *pdtl = fopen(dtl_file, "w");

    if (*pdtl == NULL) {
        fprintf(stderr, "%s:  Cannot open \"%s\" for text writing.\n",
                program_name, dtl_file);
        exit(EXIT_FAILURE);
    }

    return 1; /* OK */
} /* open_dtl */

int dv2dt(FILE* dvi, FILE* dtl) {
    int opcode;
    COUNT count; /* intended to count bytes to DVI file; as yet unused. */

    PRINT_BCOM;
    fprintf(dtl, "variety ");
    /*  fprintf (dtl, BMES); */
    fprintf(dtl, VARIETY);
    /*  fprintf (dtl, EMES); */
    PRINT_ECOM;
    fprintf(dtl, "\n");

    /* start counting DVI bytes */
    count = 0;
    while ((opcode = fgetc(dvi)) != EOF) {
        PRINT_BCOM; /* start of command and parameters */
        if (opcode < 0 || opcode > 255) {
            count += 1;
            fprintf(stderr, "%s:  Non-byte from \"fgetc()\"!\n", program_name);
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
            count += special(dvi, dtl, opcode - 238);
        } else if (opcode >= 243 && opcode <= 246) {
            count += fontdef(dvi, dtl, opcode - 242);
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
            fprintf(stderr, "%s:  unknown byte.\n", program_name);
            exit(EXIT_FAILURE);
        }
        PRINT_ECOM; /* end of command and parameters */
        fprintf(dtl, "\n");
        if (fflush(dtl) == EOF) {
            fprintf(stderr, "%s:  fflush on dtl file gave write error!\n",
                    program_name);
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
        fprintf(stderr,
                "%s:  read_unsigned() asked for %d bytes.  Must be 1 to 4.\n",
                program_name, nBytes);
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

    fprintf(dtl, " ");
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
        fprintf(stderr, 
                "%s:  read_signed() asked for %d bytes.  Must be 1 to 4.\n",
                program_name, nBytes);
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

    fprintf(dtl, " ");
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
        fprintf(stderr, "%s: opcode %d is outside table %s [ %d to %d ] !\n",
                program_name, opcode, table.name, table.first, table.last);
        exit(EXIT_FAILURE);
    }

    op = table.list[opcode - table.first];

    /* Further defensive programming. */
    if (op.code != opcode) {
        fprintf(stderr, "%s: internal table %s wrong!\n", 
                program_name, table.name);
        exit(EXIT_FAILURE);
    }

    bytes_count = 1;
    fprintf(dtl, "%s", op.name);

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
            fprintf(stderr, "%s: internal read of table %s failed!\n",
                    program_name, table.name);
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
    fprintf(dtl, BSEQ);

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
    fprintf(dtl, ESEQ);

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
            fprintf(dtl, "%c", ESC_CHAR);
            fprintf(dtl, "%c", ESC_CHAR);
            break;
        case QUOTE_CHAR:
            fprintf(dtl, "%c", ESC_CHAR);
            fprintf(dtl, "%c", QUOTE_CHAR);
            break;
        case BSEQ_CHAR:
            fprintf(dtl, "%c", ESC_CHAR);
            fprintf(dtl, "%c", BSEQ_CHAR);
            break;
        case ESEQ_CHAR:
            fprintf(dtl, "%c", ESC_CHAR);
            fprintf(dtl, "%c", ESEQ_CHAR);
            break;
        default: 
            fprintf(dtl, "%c", charcode); 
            break;
    }
} /* set_pchar */

/** copy string of k characters from dvi file to dtl file
 *
 *  @param[in]  k
 *  @param[in]  dvi
 *  @param[out] dtl
 */
void xfer_string(int k, FILE* dvi, FILE* dtl) {
    fprintf(dtl, " ");
    fprintf(dtl, "'");

    for (int i = 0; i < k; i++) {
        int ch = fgetc(dvi);

        if (ch == ESC_CHAR || ch == EMES_CHAR) {
            fprintf(dtl, "%c", ESC_CHAR);
        }
        fprintf(dtl, "%c", ch);
    }

    fprintf(dtl, "'");
} /* xfer_string */

/* read special 1 .. 4 from dvi and write in dtl */
/* return number of DVI bytes interpreted into DTL */
COUNT special(FILE* dvi, FILE* dtl, int n) {
    U4 k;

    if (n < 1 || n > 4) {
        fprintf(stderr, "%s:  special %d, range is 1 to 4.\n", program_name, n);
        exit(EXIT_FAILURE);
    }

    fprintf(dtl, "%s%d", SPECIAL, n);

    /* k[n] = length of special string */
    k = xref_unsigned(n ,dvi, dtl);

    /* x[k] = special string */
    xfer_string(k, dvi, dtl);

    return (1 + n + k);
}
/* end special */

/* read fontdef 1 .. 4 from dvi and write in dtl */
/* return number of DVI bytes interpreted into DTL */
COUNT fontdef(FILE* dvi, FILE* dtl, int n) {
    U4 c, a, l;

    if (n < 1 || n > 4) {
        fprintf(stderr, "%s:  font def %d, range is 1 to 4.\n", program_name, n);
        exit(EXIT_FAILURE);
    }

    fprintf(dtl, "%s%d", FONTDEF, n);

    /* k[n] = font number */
    if (n == 4) {
        xref_signed(n, dvi, dtl);
    } else {
        xref_unsigned(n, dvi, dtl);
    }

    /* c[4] = checksum */
    fprintf(dtl, " ");
    c = read_unsigned(4, dvi);

#ifdef HEX_CHECKSUM
    fprintf(dtl, HEX_FMT, c);
#else /* NOT HEX_CHECKSUM */
    /* write in octal, to allow quick comparison with tftopl's output */
    fprintf(dtl, OCT_FMT, c);
#endif

    /* s[4] = scale factor */
    xref_unsigned(4, dvi, dtl);

    /* d[4] = design size */
    xref_unsigned(4, dvi, dtl);

    /* a[1] = length of area (directory) name */
    a = read_unsigned(1, dvi);
    fprintf(dtl, " ");
    fprintf(dtl, U4_FMT, a);

    /* l[1] = length of font name */
    l = read_unsigned(1, dvi);
    fprintf(dtl, " ");
    fprintf(dtl, U4_FMT, l);

    /* n[a+l] = font pathname string => area (directory) + font */
    xfer_string(a, dvi, dtl);
    xfer_string(l, dvi, dtl);

    return (1 + n + 4 + 4 + 4 + 1 + 1 + a + l);
}
/* end fontdef */

/* read preamble from dvi and write in dtl */
/* return number of DVI bytes interpreted into DTL */
COUNT preamble(FILE* dvi, FILE* dtl) {
    U4 k;

    fprintf(dtl, "pre");

    /* i[1] = DVI format identification */
    xref_unsigned(1, dvi, dtl);

    /* num[4] = numerator of DVI unit */
    xref_unsigned(4, dvi, dtl);

    /* den[4] = denominator of DVI unit */
    xref_unsigned(4, dvi, dtl);

    /* mag[4] = 1000 x magnification */
    xref_unsigned(4, dvi, dtl);

    /* k[1] = length of comment */
    k = xref_unsigned(1, dvi, dtl);

    /* x[k] = comment string */
    xfer_string(k, dvi, dtl);

    return (1 + 1 + 4 + 4 + 4 + 1 + k);
} /* end preamble */

/* read postamble from dvi and write in dtl */
/* return number of bytes */
COUNT postamble(FILE* dvi, FILE* dtl) {
    fprintf(dtl, "post");

    /* p[4] = pointer to final bop */
    xref_unsigned(4, dvi, dtl);

    /* num[4] = numerator of DVI unit */
    xref_unsigned(4, dvi, dtl);

    /* den[4] = denominator of DVI unit */
    xref_unsigned(4, dvi, dtl);

    /* mag[4] = 1000 x magnification */
    xref_unsigned(4, dvi, dtl);

    /* l[4] = height + depth of tallest page */
    xref_unsigned(4, dvi, dtl);

    /* u[4] = width of widest page */
    xref_unsigned(4, dvi, dtl);

    /* s[2] = maximum stack depth */
    xref_unsigned(2, dvi, dtl);

    /* t[2] = total number of pages (bop commands) */
    xref_unsigned(2, dvi, dtl);

    /*  return (29);  */
    return (1 + 4 + 4 + 4 + 4 + 4 + 4 + 2 + 2);
} /* end postamble */

/* read post_post from dvi and write in dtl */
/* return number of bytes */
COUNT postpost(FILE* dvi, FILE* dtl) {
    int b223; /* hope this is 8-bit clean */
    int n223; /* number of "223" bytes in final padding */

    fprintf(dtl, "post_post");

    /* q[4] = pointer to post command */
    xref_unsigned(4, dvi, dtl);

    /* i[1] = DVI identification byte */
    xref_unsigned(1, dvi, dtl);

    /* final padding by "223" bytes */
    /* hope this way of obtaining b223 is 8-bit clean */
    for (n223 = 0; (b223 = fgetc(dvi)) == 223; n223++) {
        fprintf(dtl, " ");
        fprintf(dtl, "%d", 223);
    }
    if (n223 < 4) {
        fprintf(stderr, "%s:  bad post_post:  fewer than four \"223\" bytes.\n",
                program_name);
        exit(EXIT_FAILURE);
    }
    if (b223 != EOF) {
        fprintf(stderr, "%s:  bad post_post:  doesn't end with a \"223\".\n",
                program_name);
        exit(EXIT_FAILURE);
    }

    return (1 + 4 + 1 + n223);
}
/* end postpost */

/* end of "dv2dt.c" */
