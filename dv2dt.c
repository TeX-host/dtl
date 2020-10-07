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
#include "dv2dt.h"


int main(int argc, char* argv[]) {
    FILE* dvi = stdin;
    FILE* dtl = stdout;

    /* Watch out:  C's standard library's string functions are dicey */
    strncpy(program, argv[0], MAXSTRLEN);

    if (argc > 1) open_dvi(argv[1], &dvi);

    if (argc > 2) open_dtl(argv[2], &dtl);

    dv2dt(dvi, dtl);

    return 0; /* OK */
}
/* end main */

/* I:  dvi_file;  I:  pdvi;  O:  *pdvi. */
int open_dvi(char* dvi_file, FILE** pdvi) {
    if (pdvi == NULL) {
        fprintf(stderr, "%s:  address of dvi variable is NULL.\n", program);
        exit(1);
    }

    *pdvi = fopen(dvi_file, "rb");

    if (*pdvi == NULL) {
        fprintf(stderr, "%s:  Cannot open \"%s\" for binary reading.\n",
                program, dvi_file);
        exit(1);
    }

    return 1; /* OK */
}
/* open_dvi */

/* I:  dtl_file;  I:  pdtl;  O:  *pdtl. */
int open_dtl(char* dtl_file, FILE** pdtl) {
    if (pdtl == NULL) {
        fprintf(stderr, "%s:  address of dtl variable is NULL.\n", program);
        exit(1);
    }

    *pdtl = fopen(dtl_file, "w");

    if (*pdtl == NULL) {
        fprintf(stderr, "%s:  Cannot open \"%s\" for text writing.\n", program,
                dtl_file);
        exit(1);
    }

    return 1; /* OK */
}
/* open_dtl */

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
            fprintf(stderr, "%s:  Non-byte from \"fgetc()\"!\n", program);
            exit(1);
        } else if (opcode <= 127) {
            /* setchar commands */
            /* count += 1; */
            /* fprintf (dtl, "%s%d", SETCHAR, opcode); */
            count += setseq(opcode, dvi, dtl);
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
            fprintf(stderr, "%s:  unknown byte.\n", program);
            exit(1);
        }
        PRINT_ECOM; /* end of command and parameters */
        fprintf(dtl, "\n");
        if (fflush(dtl) == EOF) {
            fprintf(stderr, "%s:  fflush on dtl file gave write error!\n",
                    program);
            exit(1);
        }
    } /* end while */

    return 1; /* OK */
}
/* dv2dt */


COUNT write_unsigned(int n, FILE* dvi, FILE* dtl) {
    U4 unum;

    fprintf(dtl, " ");
    unum = read_unsigned(n, dvi);
    fprintf(dtl, U4_FMT, unum);
    return n;
} /* end write_unsigned */

COUNT write_signed(int n, FILE* dvi, FILE* dtl) {
    S4 snum;

    fprintf(dtl, " ");
    snum = read_signed(n, dvi);
    fprintf(dtl, S4_FMT, snum);
    return n;
} /* end write_signed */

/* read 1 <= n <= 4 bytes for an unsigned integer from dvi file */
/* DVI format uses Big-endian storage of numbers. */
U4 read_unsigned(int n, FILE* dvi) {
    U4 integer;
    int ibyte = 0;
    int i;

    if (n < 1 || n > 4) {
        fprintf(stderr,
                "%s:  read_unsigned() asked for %d bytes.  Must be 1 to 4.\n",
                program, n);
        exit(1);
    }

    /* Following calculation works iff storage is big-endian. */
    integer = 0;
    for (i = 0; i < n; i++) {
        integer *= 256;
        ibyte = fgetc(dvi);
        integer += ibyte;
    }

    return integer;
} /* end read_unsigned */

/* read 1 <= n <= 4 bytes for a signed integer from dvi file */
/* DVI format uses Big-endian storage of numbers. */
S4 read_signed(int n, FILE* dvi) {
    S4 integer;
    int ibyte = 0;
    int i;

    if (n < 1 || n > 4) {
        fprintf(stderr, "%s:  read_signed() asked for %d bytes.  Must be 1 to 4.\n",
                program, n);
        exit(1);
    }

    /* Following calculation works iff storage is big-endian. */
    integer = 0;
    for (i = 0; i < n; i++) {
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

/* write command with given opcode in given table */
/* return number of DVI bytes in this command */
COUNT write_table(op_table table, int opcode, FILE* dvi, FILE* dtl) {
    op_info op;       /* pointer into table of operations and arguments */
    COUNT bcount = 0; /* number of bytes in arguments of this opcode */
    String args;      /* arguments string */
    int i;            /* count of arguments read from args */
    int pos;          /* position in args */

    /* Defensive programming. */
    if (opcode < table.first || opcode > table.last) {
        fprintf(stderr, "%s: opcode %d is outside table %s [ %d to %d ] !\n",
                program, opcode, table.name, table.first, table.last);
        exit(1);
    }

    op = table.list[opcode - table.first];

    /* Further defensive programming. */
    if (op.code != opcode) {
        fprintf(stderr, "%s: internal table %s wrong!\n", program, table.name);
        exit(1);
    }

    bcount = 1;
    fprintf(dtl, "%s", op.name);

    /* NB:  sscanf does an ungetc, */
    /*      so args must be writable. */

    strncpy(args, op.args, MAXSTRLEN);

    pos = 0;
    for (i = 0; i < op.nargs; i++) {
        int argtype; /* sign and number of bytes in current argument */
        int nconv;   /* number of successful conversions from args */
        int nread;   /* number of bytes read from args */

        nconv = sscanf(args + pos, "%d%n", &argtype, &nread);

        /* internal consistency checks */
        if (nconv != 1 || nread <= 0) {
            fprintf(stderr, "%s: internal read of table %s failed!\n", program,
                    table.name);
            exit(1);
        }

        pos += nread;

        bcount += (argtype < 0 ? write_signed(-argtype, dvi, dtl)
                               : write_unsigned(argtype, dvi, dtl));
    } /* end for */

    return bcount;
} /* write_table */

/* write a sequence of setchar commands */
/* return count of DVI bytes interpreted into DTL */
COUNT setseq(int opcode, FILE* dvi, FILE* dtl) {
    int charcode = opcode; /* fortuitous */
    int ccount = 0;

    if (!isprint(charcode)) {
        ccount = 1;
        fprintf(dtl, "%s%02X", SETCHAR, opcode);
    } else {
        /* start of sequence of font characters */
        fprintf(dtl, BSEQ);

        /* first character */
        ccount = 1;
        setpchar(charcode, dtl);

        /* subsequent characters */
        while ((opcode = fgetc(dvi)) != EOF) {
            if (opcode < 0 || opcode > 127) {
                break; /* not a setchar command, so sequence has ended */
            }
            charcode = opcode;      /* fortuitous */
            if (!isprint(charcode)) /* not printable ascii */
            {
                break; /* end of font character sequence, as for other commands
                        */
            } else     /* printable ASCII */
            {
                ccount += 1;
                setpchar(charcode, dtl);
            }
        } /* end for loop */

        /* prepare to reread opcode of next DVI command */
        if (ungetc(opcode, dvi) == EOF) {
            fprintf(stderr, "setseq:  cannot push back a byte\n");
            exit(1);
        }

        /* end of sequence of font characters */
        fprintf(dtl, ESEQ);
    }
    return ccount;
}
/* setseq */

/* set printable character */
void setpchar(int charcode, FILE* dtl) {
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
        default: fprintf(dtl, "%c", charcode); break;
    }
}
/* setpchar */

/* copy string of k characters from dvi file to dtl file */
void xferstring(int k, FILE* dvi, FILE* dtl) {
    int i;
    int ch;

    fprintf(dtl, " ");
    fprintf(dtl, "'");
    for (i = 0; i < k; i++) {
        ch = fgetc(dvi);
        if (ch == ESC_CHAR || ch == EMES_CHAR) {
            fprintf(dtl, "%c", ESC_CHAR);
        }
        fprintf(dtl, "%c", ch);
    }
    fprintf(dtl, "'");
}
/* xferstring */

/* read special 1 .. 4 from dvi and write in dtl */
/* return number of DVI bytes interpreted into DTL */
COUNT special(FILE* dvi, FILE* dtl, int n) {
    U4 k;

    if (n < 1 || n > 4) {
        fprintf(stderr, "%s:  special %d, range is 1 to 4.\n", program, n);
        exit(1);
    }

    fprintf(dtl, "%s%d", SPECIAL, n);

    /* k[n] = length of special string */
    fprintf(dtl, " ");
    k = read_unsigned(n, dvi);
    fprintf(dtl, U4_FMT, k);

    /* x[k] = special string */
    xferstring(k, dvi, dtl);

    return (1 + n + k);
}
/* end special */

/* read fontdef 1 .. 4 from dvi and write in dtl */
/* return number of DVI bytes interpreted into DTL */
COUNT fontdef(FILE* dvi, FILE* dtl, int n) {
    U4 ku, c, s, d, a, l;
    S4 ks;

    if (n < 1 || n > 4) {
        fprintf(stderr, "%s:  font def %d, range is 1 to 4.\n", program, n);
        exit(1);
    }

    fprintf(dtl, "%s%d", FONTDEF, n);

    /* k[n] = font number */
    fprintf(dtl, " ");
    if (n == 4) {
        ks = read_signed(n, dvi);
        fprintf(dtl, S4_FMT, ks);
    } else {
        ku = read_unsigned(n, dvi);
        fprintf(dtl, U4_FMT, ku);
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
    fprintf(dtl, " ");
    s = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, s);

    /* d[4] = design size */
    fprintf(dtl, " ");
    d = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, d);

    /* a[1] = length of area (directory) name */
    a = read_unsigned(1, dvi);
    fprintf(dtl, " ");
    fprintf(dtl, U4_FMT, a);

    /* l[1] = length of font name */
    l = read_unsigned(1, dvi);
    fprintf(dtl, " ");
    fprintf(dtl, U4_FMT, l);

    /* n[a+l] = font pathname string => area (directory) + font */
    xferstring(a, dvi, dtl);
    xferstring(l, dvi, dtl);

    return (1 + n + 4 + 4 + 4 + 1 + 1 + a + l);
}
/* end fontdef */

/* read preamble from dvi and write in dtl */
/* return number of DVI bytes interpreted into DTL */
COUNT preamble(FILE* dvi, FILE* dtl) {
    U4 id, num, den, mag, k;

    fprintf(dtl, "pre");

    /* i[1] = DVI format identification */
    fprintf(dtl, " ");
    id = read_unsigned(1, dvi);
    fprintf(dtl, U4_FMT, id);

    /* num[4] = numerator of DVI unit */
    fprintf(dtl, " ");
    num = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, num);

    /* den[4] = denominator of DVI unit */
    fprintf(dtl, " ");
    den = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, den);

    /* mag[4] = 1000 x magnification */
    fprintf(dtl, " ");
    mag = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, mag);

    /* k[1] = length of comment */
    fprintf(dtl, " ");
    k = read_unsigned(1, dvi);
    fprintf(dtl, U4_FMT, k);

    /* x[k] = comment string */
    xferstring(k, dvi, dtl);

    return (1 + 1 + 4 + 4 + 4 + 1 + k);
}
/* end preamble */

/* read postamble from dvi and write in dtl */
/* return number of bytes */
COUNT postamble(FILE* dvi, FILE* dtl) {
    U4 p, num, den, mag, l, u, s, t;

    fprintf(dtl, "post");

    /* p[4] = pointer to final bop */
    fprintf(dtl, " ");
    p = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, p);

    /* num[4] = numerator of DVI unit */
    fprintf(dtl, " ");
    num = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, num);

    /* den[4] = denominator of DVI unit */
    fprintf(dtl, " ");
    den = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, den);

    /* mag[4] = 1000 x magnification */
    fprintf(dtl, " ");
    mag = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, mag);

    /* l[4] = height + depth of tallest page */
    fprintf(dtl, " ");
    l = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, l);

    /* u[4] = width of widest page */
    fprintf(dtl, " ");
    u = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, u);

    /* s[2] = maximum stack depth */
    fprintf(dtl, " ");
    s = read_unsigned(2, dvi);
    fprintf(dtl, U4_FMT, s);

    /* t[2] = total number of pages (bop commands) */
    fprintf(dtl, " ");
    t = read_unsigned(2, dvi);
    fprintf(dtl, U4_FMT, t);

    /*  return (29);  */
    return (1 + 4 + 4 + 4 + 4 + 4 + 4 + 2 + 2);
}
/* end postamble */

/* read post_post from dvi and write in dtl */
/* return number of bytes */
COUNT postpost(FILE* dvi, FILE* dtl) {
    U4 q, id;
    int b223; /* hope this is 8-bit clean */
    int n223; /* number of "223" bytes in final padding */

    fprintf(dtl, "post_post");

    /* q[4] = pointer to post command */
    fprintf(dtl, " ");
    q = read_unsigned(4, dvi);
    fprintf(dtl, U4_FMT, q);

    /* i[1] = DVI identification byte */
    fprintf(dtl, " ");
    id = read_unsigned(1, dvi);
    fprintf(dtl, U4_FMT, id);

    /* final padding by "223" bytes */
    /* hope this way of obtaining b223 is 8-bit clean */
    for (n223 = 0; (b223 = fgetc(dvi)) == 223; n223++) {
        fprintf(dtl, " ");
        fprintf(dtl, "%d", 223);
    }
    if (n223 < 4) {
        fprintf(stderr, "%s:  bad post_post:  fewer than four \"223\" bytes.\n",
                program);
        exit(1);
    }
    if (b223 != EOF) {
        fprintf(stderr, "%s:  bad post_post:  doesn't end with a \"223\".\n",
                program);
        exit(1);
    }

    return (1 + 4 + 1 + n223);
}
/* end postpost */

/* end of "dv2dt.c" */
