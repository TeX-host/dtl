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

#include "dt2dv.h"


/** Main functions.
 *
 * ## global var
 *  @param[in] prog_name
 *  @param[in] nfile
 *  @param[in] dtl_fp
 *  @param[in] dvi_fp
 *  @param[in] dtl_filename
 *  @param[in] dvi_filename
 */
int main(int argc, char* argv[]) {
    void (*handler)(int); /* Previous signal handler */
    int i;

    prog_name = argv[0]; /* name of this program */

    /* memory violation signal handler */
    handler = (void (*)(int))signal(SIGSEGV, mem_viol);

    /* message about program and compiler */
    /* NB:  LTU EE's Sun/OS library is BSD, even though gcc 2.2.2 is ANSI */
    fprintf(stderr, "\n");
    fprintf(stderr, "Program \"%s\" version %s compiled %s %s in standard C.\n",
            prog_name, VERSION, __DATE__, __TIME__);

    /* interpret command line arguments */
    nfile = 0;
    dtl_fp = dvi_fp = NULL;
    dtl_filename = dvi_filename = "";

    for (i = 1; i < argc; i++) {
        /* parse options, followed by any explicit filenames */
        parse(argv[i]);
    }

    if (nfile != 2) {
        /* not exactly two files specified, so give help */
        give_help();
    } else {
        /* the real works */
        dt2dv(dtl_fp, dvi_fp);
    }

    return 0; /* OK */
} /* end main */

/**
 * @param[in] dtl_line
 */
void mem_viol(int sig) {
    void (*handler)(int);
    handler = (void (*)(int))signal(SIGSEGV, mem_viol);
    if (sig != SIGSEGV) {
        PRINT_PROGNAME;
        fprintf(stderr, "(mem_viol) : called with wrong signal!\n");
    }
    PRINT_PROGNAME;
    fprintf(stderr, "(mem_viol) : RUNTIME MEMORY ERROR : memory violation, ");
    fprintf(stderr, "dtl line >= ");
    fprintf(stderr, COUNT_FMT, dtl_line.num);
    fprintf(stderr, "\n");
    dexit(1);
} /* mem_viol */

/** Show a help msg.
 * 
 * @param[in] opts[]
 */
void give_help(void) {
    char* keyword;

    fprintf(stderr, "usage:   ");
    PRINT_PROGNAME;
    fprintf(stderr, "[options]  dtl_file  dvi_file");
    fprintf(stderr, "\n");

    for (int i = 0; (keyword = opts[i].keyword) != NULL; i++) {
        fprintf(stderr, "    ");
        fprintf(stderr, "[%s]", keyword);
        fprintf(stderr, "    ");
        fprintf(stderr, "%s", opts[i].desc);
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "Messages, like this one, go to stderr.\n");
} /* give_help */

/* do nothing */
void no_op(void) {}

/** Use stdin as dtl file.
 *
 * ## global var
 *  @param[out] dtl_fp
 *  @param[out] dtl_filename
 *  @param[out] nfile
 */
void dtl_stdin(void) {
    dtl_fp = stdin;
    dtl_filename = "Standard Input";
    ++nfile;
} /* dtl_stdin */

/** Use stdout as dvi file.
 *
 * ## global var
 *  @param[out] dvi_fp
 *  @param[out] dvi_filename
 *  @param[out] nfile
 */
void dvi_stdout(void) {
    /* ! Perilous to monitors! */
    dvi_fp = stdout;
    dvi_filename = "Standard Output";
    ++nfile;
} /* dvi_stdout */


/** parse one command-line argument, `s'
 *
 * ## global var
 *  @param[in] opts[]
 */
int parse(char* s) {
    int i;
    char* keyword;

    for (i = 0; (keyword = opts[i].keyword) != NULL; i++) {
        if (strncmp(s, keyword, strlen(keyword)) == 0) {
            void (*pfn)(void);

            (*(opts[i].p_var)) = 1; /* turn option on */
            if ((pfn = opts[i].p_fn) != NULL) {
                (*pfn)(); /* call option function */
            }
            return i;
        }
    }

    /* reached here, so not an option: assume it's a filename */
    process(s);

    return i;
} /* parse */


/** Open dtl file, save pointer to *pdtl.
 *
 *  @param[in] dtl_file
 *  @param[inout] pdtl
 *
 *  @param[out] dtl_filename [global]
 */
int open_dtl(char* dtl_file, FILE** pdtl) {
    dtl_filename = dtl_file;

    if (dtl_filename == NULL) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(open_dtl) : INTERNAL ERROR : dtl file's name is NULL.\n");
        dexit(1);
    }

    if (pdtl == NULL) {
        PRINT_PROGNAME;
        fprintf(
            stderr,
            "(open_dtl) : INTERNAL ERROR : address of dtl variable is NULL.\n");
        dexit(1);
    }

    *pdtl = fopen(dtl_file, "r");
    if (*pdtl == NULL) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(open_dtl) : DTL FILE ERROR : Cannot open \"%s\" for text "
                "reading.\n",
                dtl_file);
        dexit(1);
    }

    return 1; /* OK */
} /* open_dtl */

/** Open dvi file, save pointer to *pdvi.
 *
 *  @param[in] dvi_file
 *  @param[inout] pdvi
 *
 *  @param[out] dvi_filename [global]
 */
int open_dvi(char* dvi_file, FILE** pdvi) {
    dvi_filename = dvi_file;

    if (dvi_filename == NULL) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(open_dvi) : INTERNAL ERROR : dvi file's name is NULL.\n");
        dexit(1);
    }

    if (pdvi == NULL) {
        PRINT_PROGNAME;
        fprintf(
            stderr,
            "(open_dvi) : INTERNAL ERROR : address of dvi variable is NULL.\n");
        dexit(1);
    }

    *pdvi = fopen(dvi_file, "wb");
    if (*pdvi == NULL) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(open_dvi) : DVI FILE ERROR : Cannot open \"%s\" for binary "
                "writing.\n",
                dvi_file);
        dexit(1);
    }

    return 1; /* OK */
} /* open_dvi */

/**
 *
 * ## global var
 *  @param[inout] dtl_fp
 *  @param[inout] dvi_fp
 *  @param[out] nfile
 */
void process(char* s) {
    if (dtl_fp == NULL) {
        /* first filename assumed to be DTL input */
        open_dtl(s, &dtl_fp);
    } else if (dvi_fp == NULL) {
        /* second filename assumed to be DVI output */
        open_dvi(s, &dvi_fp);
    } else {
        PRINT_PROGNAME;
        fprintf(stderr, "(process) : at most two filenames allowed.\n");
        exit(1);
    }

    ++nfile;
} /* process */


/* write byte into dvi file */
int put_byte(int byte, FILE* dvi) {
    check_byte(byte);

    /*  if (fprintf (dvi, "%c", byte) != 1) */
    if (fprintf(dvi, "%c", byte) < 0) {
        PRINT_PROGNAME;
        fprintf(
            stderr,
            "(put_byte) : DVI FILE ERROR (%s) : cannot write to dvi file.\n",
            dtl_filename);
        dexit(1);
    }
    ++dvi_written;

    return 1; /* OK */
} /* put_byte */

/**
 *
 * ## global var
 *  + dtl_line
 */
int dt2dv(FILE* dtl, FILE* dvi) {
    int nprefixes = 0;         /* number of prefixes in cmd_prefixes[] list. */
    static Token dtl_cmd = ""; /* DTL command name */
    COUNT nread = 0; /* number of bytes read by a function from dtl file. */

    nprefixes = sizeof(cmd_prefixes) / sizeof(CmdPrefix);

    /* Construct array of all NCMDS == 256 DTL commands */
    (void)cons_cmds(nprefixes, cmd_prefixes, cmd_table);

    /* DTL commands have the form "[ ]*command arg ... arg[ ]*", */
    /* possibly enclosed in a BCOM, ECOM pair, */
    /* and are separated by optional whitespace, typically newlines. */
    /* That is, each command and its arguments are parenthesised, */
    /* with optional spaces after the BCOM and before the ECOM, if any. */

    /* dt2dv is now at the very start of the DTL file */
    dtl_line.num = 0;
    dtl_read = 0;

    /* The very first thing should be the "variety" signature */
    nread = read_variety(dtl);

    /* while not end of dtl file or reading error, */
    /*   read, interpret, and write commands */
    while (!feof(dtl)) {
        int opcode;

        com_read = 0;

        if (group) {
            /* BCOM check */
            static Token token = ""; /* DTL token */
            nread = read_token(dtl, token);
            /* test for end of input, or reading error */
            if (strlen(token) == 0) {
                if (debug) {
                    PRINT_PROGNAME;
                    fprintf(stderr,
                            "(dt2dv) : end of input, or reading error.\n");
                }
                break;
            }
            /* test whether this command begins correctly */
            else if (strcmp(token, BCOM) != 0) {
                PRINT_PROGNAME;
                fprintf(stderr,
                        "(dt2dv) : DTL FILE ERROR (%s) : ", dtl_filename);
                fprintf(stderr, "command must begin with \"%s\", ", BCOM);
                fprintf(stderr, "not `%c' (char %d).\n", token[0], token[0]);
                dexit(1);
            }
            /* end BCOM check */
        }

        /* read the command name */
        nread = read_token(dtl, dtl_cmd);
        /* test for end of input, or reading error */
        if (strlen(dtl_cmd) == 0) {
            if (debug) {
                PRINT_PROGNAME;
                fprintf(stderr, "(dt2dv) : end of input, or reading error.\n");
            }
            break;
        } else {
            if (debug) {
                PRINT_PROGNAME;
                fprintf(stderr, "(dt2dv) : command ");
                fprintf(stderr, COUNT_FMT, ncom);
                fprintf(stderr, " = \"%s\".\n", dtl_cmd);
            }

            /* find opcode for this command */
            if (find_command(dtl_cmd, &opcode) == 1) {
                /* write the opcode, if we can */
                put_byte(opcode, dvi);

                /* treat the arguments, if any */
                xfer_args(dtl, dvi, opcode);
            } else if (dtl_cmd[0] == BSEQ_CHAR) {
                /* sequence of font characters for SETCHAR */
                set_seq(dtl, dvi);
            } else {
                PRINT_PROGNAME;
                fprintf(
                    stderr,
                    "(dt2dv) : DTL FILE ERROR (%s) : unknown command \"%s\".\n",
                    dtl_filename, dtl_cmd);
                dexit(1);
            }
        }

        if (group) {
            /* seek ECOM after command's last argument and optional whitespace
             */
            static Token token = ""; /* DTL token */
            nread = read_token(dtl, token);
            /* test for end of input, or reading error */
            if (strlen(token) == 0) {
                if (debug) {
                    PRINT_PROGNAME;
                    fprintf(stderr,
                            "(dt2dv) : end of input, or reading error.\n");
                }
                break;
            }
            if (strcmp(token, ECOM) != 0) {
                PRINT_PROGNAME;
                fprintf(stderr,
                        "(dt2dv) : DTL FILE ERROR (%s) : ", dtl_filename);
                fprintf(stderr, "ECOM (\"%s\") expected, not `%c' (char %d).\n",
                        ECOM, token[0], token[0]);
                dexit(1);
            }
            /* end ECOM check */
        }

        ++ncom; /* one more command successfully read and interpreted */
    }
    /* end while */

    PRINT_PROGNAME;
    fprintf(stderr, "(dt2dv) :\n");
    fprintf(stderr, "Read (from file \"%s\") ", dtl_filename);
    fprintf(stderr, COUNT_FMT, dtl_read);
    fprintf(stderr, " DTL bytes (");
    fprintf(stderr, COUNT_FMT, dtl_line.num);
    fprintf(stderr, " lines);\n");
    fprintf(stderr, "wrote (to file \"%s\") ", dvi_filename);
    fprintf(stderr, COUNT_FMT, dvi_written);
    fprintf(stderr, " DVI bytes;\n");
    fprintf(stderr, "completely interpreted ");
    fprintf(stderr, COUNT_FMT, ncom);
    fprintf(stderr, " DVI command%s.\n", (ncom == 1 ? "" : "s"));
    fprintf(stderr, "\n");

    free_cmds(cmd_table);

    return 1; /* OK */
} /* dt2dv */

/// used by: cons_cmds[2], init_lstr[1], alloc_lstr[1].
void* gmalloc(size_t size) {
    void* p = NULL;

    if (size < 1) {
        PRINT_PROGNAME;
        fprintf(stderr, "(gmalloc) : INTERNAL ERROR : ");
        fprintf(stderr, "unreasonable request to malloc %zd bytes\n", size);
        dexit(1);
    }

    p = malloc(size);
    if (p == NULL) {
        PRINT_PROGNAME;
        fprintf(stderr, "(gmalloc) : MEMORY ALLOCATION ERROR : ");
        fprintf(stderr, "operating system failed to malloc %zd bytes\n", size);
        dexit(1);
    }
    return (p);
} /* gmalloc */


void dinfo(void) {
    PRINT_PROGNAME;
    fprintf(stderr, "(dinfo) : ");
    fprintf(stderr, "Current DTL input line ");
    fprintf(stderr, COUNT_FMT, dtl_line.num);
    fprintf(stderr, " :\n");
    fprintf(stderr, "\"%s\"\n", dtl_line.buf);
    fprintf(stderr, "Read ");
    fprintf(stderr, COUNT_FMT, dtl_read);
    fprintf(stderr, " DTL bytes (");
    fprintf(stderr, COUNT_FMT, com_read);
    fprintf(stderr, " in current command), wrote ");
    fprintf(stderr, COUNT_FMT, dvi_written);
    fprintf(stderr, " DVI bytes.\n");
    fprintf(stderr, "Successfully interpreted ");
    fprintf(stderr, COUNT_FMT, ncom);
    fprintf(stderr, " DVI command%s.\n", (ncom == 1 ? "" : "s"));
} /* dinfo */


void dexit(int n) {
    dinfo();
    PRINT_PROGNAME;
    fprintf(stderr, "(dexit) : exiting with status %d.\n", n);
    exit(n);
} /* dexit */


int cons_cmds(int nprefixes, CmdPrefix prefix[], CmdTable cmds) {
    int code;        /* first opcode for a given command prefix */
    int opcode;      /* command's opcode */
    int nsuffixes;   /* number of commands with a given prefix */
    int isuffix;     /**** integer suffix, of at most three digits ****/
    String suffix;   /* suffix string generated from integer suffix */
    size_t plen = 0; /* prefix length */
    size_t slen;     /* suffix length */
    size_t clen;     /* whole command name length */
    int i, j;        /* loop indices */

    for (i = 0; i < nprefixes; prefix++, i++) {
        code = prefix->first_code;
        if (code < 0 || code > 255) {
            PRINT_PROGNAME;
            fprintf(stderr, "(cons_cmds) : INTERNAL ERROR : ");
            fprintf(
                stderr,
                "prefix listed internally with code = %d, must be 0 to 255\n",
                code);
            dexit(1);
        }

        if (prefix->has_suffix) {
            plen = strlen(prefix->name);
            /**** Suffixes in DTL are Integers, in Sequence */
            if (prefix->last_suffix < prefix->first_suffix) {
                PRINT_PROGNAME;
                fprintf(stderr, "(cons_cmds) : INTERNAL ERROR : ");
                fprintf(stderr, "prefix's last suffix %d < first suffix (%d)\n",
                        prefix->last_suffix, prefix->first_suffix);
                dexit(1);
            }

            nsuffixes = prefix->last_suffix - prefix->first_suffix + 1;
            opcode = prefix->first_code;
            for (j = 0; j < nsuffixes; j++, opcode++) {
                isuffix = prefix->first_suffix + j;
                if (0 <= code && code <= 127) { /* SETCHAR */
                    /* SETCHAR's suffix is written in uppercase hexadecimal */
                    sprintf(suffix, "%02X", isuffix);
                } else { /* 128 <= code && code <= 255; other DTL commands */
                    /* other commands' suffices are written in decimal */
                    sprintf(suffix, "%d", isuffix);
                }
                slen = strlen(suffix);
                clen = plen + slen;
                cmds[opcode] = (char*)gmalloc(clen + 1);
                strcpy(cmds[opcode], prefix->name);
                strcat(cmds[opcode], suffix);
            }
        } else {
            /* command name = prefix */
            plen = strlen(prefix->name);
            clen = plen;
            opcode = prefix->first_code;
            cmds[opcode] = (char*)gmalloc(clen + 1);
            strcpy(cmds[opcode], prefix->name);
        }
    }

    return 1; /* OK */
} /* cons_cmds */

void free_cmds(CmdTable cmd_table) {
    for (int i = 0; i < NCMDS; i++){ 
        free(cmd_table[i]);
    }
} /* free_cmds */

/* read a (Line *) line from fp, return length */
/* adapted from K&R (second, alias ANSI C, edition, 1988), page 165 */
int get_line(FILE* fp, Line* line, int max) {
    if (fgets(line->buf, max, fp) == NULL)
        return 0;
    else {
        ++line->num;
        line->wrote = strlen(line->buf);
        line->read = 0;
        return 1;
    }
}
/* get_line */

/** read one character from dtl_line if possible,
 * otherwise read another dtl_line from fp
 * return 1 if a character is read, 0 if at end of fp file.
 *
 * ## global var
 *  + dtl_line
 */
int read_line_char(FILE* fp, int* ch) {
    if (dtl_line.wrote == 0 || dtl_line.read >= dtl_line.wrote) {
        int line_status;
        /* refill line buffer */
        line_status = get_line(fp, &dtl_line, MAX_LINE);
        if (line_status == 0) {
            /* at end of DTL file */
            if (debug) {
                PRINT_PROGNAME;
                fprintf(stderr, "(read_line_char) : end of DTL file\n");
                dinfo();
            }
            return 0;
        } else {
            /* new DTL line was read */
            if (debug) {
                PRINT_PROGNAME;
                fprintf(stderr, "(read_line_char) : new DTL input line:\n");
                fprintf(stderr, "\"%s\"\n", dtl_line.buf);
            }
        }
    }

    *ch = dtl_line.buf[dtl_line.read++];
    ++dtl_read;
    ++com_read; /* count another DTL command character */

    return 1;
}
/* read_line_char */

/* Read next character, if any, from file fp. */
/* Write it into *ch. */
/* If no character is read, then *ch value < 0. */
/* Return 1 if OK, 0 if EOF or error. */
int read_char(FILE* fp, int* ch) {
    int status = 1;
    int c; /* in case ch points awry, we still have something in c. */

    c = EOF;
    if (read_line_char(fp, &c) == 0) {
        /* end of fp file, or error reading it */
        status = 0;
    } else {
        if (c > 255) {
            PRINT_PROGNAME;
            fprintf(stderr,
                    "(read_char) : character %d not in range 0 to 255\n", c);
            dinfo();
            status = 0;
        } else if (!isprint(c) && !isspace(c)) {
            PRINT_PROGNAME;
            fprintf(stderr, "(read_char) : character %d %s.\n", c,
                    "not printable and not white space");
            dinfo();
            status = 0;
        }
    }
    *ch = c;

    return status;
}
/* read_char */

/* read and check DTL variety signature */
/* return number of DTL bytes written */
/* DTL variety is _NEVER_ grouped by BCOM and ECOM. */
/* Uniformity here enables the program easily to modify its behavior. */
COUNT read_variety(FILE* dtl) {
    COUNT vread = 0; /* number of DTL bytes read by read_variety */
    COUNT nread = 0; /* number of DTL bytes read by read_token */
    static Token token = "";

    /* read the DTL VARIETY keyword */
    nread = read_token(dtl, token);
    vread += nread;
    /* test whether signature begins correctly */
    if (strcmp(token, "variety") != 0) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(read_variety) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "DTL signature must begin with \"%s\", not \"%s\".\n",
                "variety", token);
        dexit(1);
    }

    /* read the DTL variety */
    nread = read_token(dtl, token);
    vread += nread;
    /* test whether variety is correct */
    if (strcmp(token, VARIETY) != 0) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(read_variety) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "DTL variety must be \"%s\", not \"%s\".\n", VARIETY,
                token);
        dexit(1);
    }

    PRINT_PROGNAME;
    fprintf(stderr, "(read_variety) : DTL variety %s is OK.\n", VARIETY);

    return vread; /* OK */
}
/* read_variety */

/** Skip whitespace characters in file fp.
 * Write in *ch the last character read from fp,
 *  or < 0 if fp could not be read.
 * Return number of characters read from fp.
 *
 * ## global var
 *  + dtl_line
 */
COUNT skip_space(FILE* fp, int* ch) {
    int c;       /* character read (if any) */
    COUNT count; /* number (0 or more) of whitespace characters read */
    int nchar;   /* number (0 or 1) of characters read by read_char */

    /* loop ends at:  end of fp file, or reading error, or not a white space */
    for (count = 0; ((nchar = read_char(fp, &c)) == 1 && isspace(c)); ++count) {
        /* otherwise, more white spaces to skip */
        if (debug) {
            /* report when each DTL end of line is reached */
            if (c == '\n') {
                PRINT_PROGNAME;
                fprintf(stderr, "(skip_space) : ");
                fprintf(stderr, "end of DTL line (at least) ");
                fprintf(stderr, COUNT_FMT, dtl_line.num);
                fprintf(stderr, "\n");
            }
        }
    }

    if (nchar == 0) {
        c = -1;
    }

    *ch = c; /* c will be < 0 if read_char could not read fp */
    return (count + nchar);
} /* skip_space */

/* read next token from dtl file. */
/* return number of DTL bytes read. */
/* A token is one of:
     a string from BMES_CHAR to the next unescaped EMES_CHAR, inclusive;
     BCOM or ECOM, unless these are empty strings;
     BSEQ or ESEQ;
     any other sequence of non-whitespace characters.
*/
COUNT read_token(FILE* dtl, char* token) {
    COUNT nread; /* number of DTL bytes read by read_token */
    int ch;      /* most recent character read */

    nread = 0;

    /* obtain first non-space character */
    /* add to nread the number of characters read from dtl by skip_space */
    nread += skip_space(dtl, &ch);

    if (ch < 0) {
        /* end of dtl file */
        /* write an empty token */
        strcpy(token, "");
        if (debug) {
            PRINT_PROGNAME;
            fprintf(stderr, "(read_token) : end of dtl file.\n");
        }
    } else if (group && ch == BCOM_CHAR) {
        strcpy(token, BCOM);
    } else if (group && ch == ECOM_CHAR) {
        strcpy(token, ECOM);
    } else {
        token[0] = ch;
        token[1] = '\0';
        if (ch == BMES_CHAR) /* string token; read until unescaped EMES_CHAR */
        {
            nread += read_mes(dtl, token + 1);
        } else if (ch == BSEQ_CHAR || ch == ESEQ_CHAR) {
            /* token is complete */
        } else /* any other string not containing (ECOM_CHAR or) whitespace */
        {
            nread += read_misc(dtl, token + 1);
        }
    }

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(read_token) : token = \"%s\"\n", token);
    }

    return (nread); /* number of bytes read from dtl file */
}
/* read_token */


CharStatus read_string_char(FILE* fp, int* ch) {
    CharStatus status = CHAR_OK; /* OK so far */
    int c;

    if (read_char(fp, &c) == 0) status = CHAR_FAIL; /* fail */

    if (c == EMES_CHAR) /* end-of-string char */
    {
        status = CHAR_EOS;    /* end of string */
    } else if (c == ESC_CHAR) /* escape character */
    {
        /* accept the next character literally, even ESC_CHAR and EMES_CHAR */
        if (read_char(fp, &c) == 0) status = CHAR_FAIL; /* fail */
    }

    *ch = c;
    return status;
}
/* read_string_char */

COUNT read_misc(FILE* fp, Token token) {
    int c;
    int count;
    /* loop ends at:  end of fp file, or reading error, or a space */
    for (count = 0; count <= MAXTOKLEN; ++count) {
        if (read_char(fp, &c) == 0 || isspace(c)) break;
        if (group && c == ECOM_CHAR) {
            (void)unread_char();
            break;
        }

        token[count] = c;
    }
    token[count] = '\0';
    return count;
}
/* read_misc */

/* called **AFTER** a BMES_CHAR has been read */
/* read file fp for characters up to next unescaped EMES_CHAR */
/* this is called a "string token" */
/* write string, including EMES_CHAR, into token[] */
/* return number of characters read from fp */
COUNT read_mes(FILE* fp, char* token) {
    COUNT dtl_count; /* number of DTL characters read by read_mes from fp */
    int more;        /* flag more == 0 to terminate loop */
    int escape;      /* flag escape == 1 if previous character was ESC_CHAR */
    int ch;          /* current DTL character */

    escape = 0;
    more = 1;
    dtl_count = 0;
    while (more) {
        if (read_char(fp, &ch) == 0) {
            /* end of fp file, or reading error */
            more = 0;
        } else /* error checking passed */
        {
            ++dtl_count;
            if (ch == EMES_CHAR && escape == 0) /* end of string */
            {
                /* include final EMES_CHAR */
                *token++ = ch;
                more = 0;
            } else if (ch == ESC_CHAR && escape == 0) {
                /* next character is not end of string */
                escape = 1;
            } else {
                /* store any other character, */
                /* including escaped EMES_CHAR and ESC_CHAR*/
                *token++ = ch;
                escape = 0;
            }
        }
    }
    *token = '\0';
    return dtl_count;
}
/* read_mes */

/** wind input back, to allow rereading of one character.
 * return 1 if this works, 0 on error.
 *
 * ## global var
 *  + dtl_line
 *  + dtl_read
 *  + com_read
 */
int unread_char(void) {
    int status;

    if (dtl_line.read > 0) {
        --dtl_line.read; /* back up one character in dtl_line */
        --dtl_read;      /* correct the count of DTL characters */
        --com_read;      /* count another DTL command character */

        status = 1; /* OK */
    } else {
        /* current DTL line is empty */
        status = 0; /* error */
    }

    return status;
} /* unread_char */

int find_command(char* command, int* opcode) {
    int found;
    int i;

    found = 0;
    for (i = 0; i < NCMDS; i++) {
        if ((cmd_table[i] != 0) && (strcmp(command, cmd_table[i]) == 0)) {
            found = 1;
            break;
        }
    }

    *opcode = i;

    return found;
}
/* find_command */

int check_byte(int byte) {
    if (byte < 0 || byte > 255) {
        PRINT_PROGNAME;
        fprintf(stderr, "(check_byte) : INTERNAL ERROR : ");
        fprintf(stderr, "byte %d not in the range of 0 to 255.\n", byte);
        dexit(1);
    }
    return 1; /* OK */
} /* check_byte */

int xfer_args(FILE* dtl, FILE* dvi, int opcode) {
    int n;

    if (opcode >= 0 && opcode <= 127)
        ; /* SETCHAR uses no data */
    else if (opcode >= 128 && opcode <= 170) {
        word_t this_bop_address = last_bop_address;

        if (opcode == 139) /* BOP */
        {
            this_bop_address = dvi_written - 1;
        }
        put_table(op_128_170, opcode, dtl, dvi);
        if (opcode == 139) /* BOP */
        {
            xfer_bop_address(dtl, dvi);
            last_bop_address = this_bop_address;
        }
    } else if (opcode >= 171 && opcode <= 234)
        ; /* FONTNUM uses no data */
    else if (opcode >= 235 && opcode <= 238)
        put_table(fnt, opcode, dtl, dvi);
    else if (opcode >= 239 && opcode <= 242) {
        n = opcode - 238;
        special(dtl, dvi, n);
    } else if (opcode >= 243 && opcode <= 246) {
        n = opcode - 242;
        fontdef(dtl, dvi, n);
    } else if (opcode == 247)
        preamble(dtl, dvi);
    else if (opcode == 248)
        postamble(dtl, dvi);
    else if (opcode == 249)
        post_post(dtl, dvi);
    else if (opcode >= 250 && opcode <= 255)
        ; /* these, undefined, opcodes use no data */
    else {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_args) : opcode %d not handled.\n", opcode);
    }

    return 1; /* OK */
}
/* xfer_args */

/* Called _after_ a BSEQ_CHAR command */
/* Read bytes from dtl file, */
/* writing corresponding SETCHAR or SET1 commands to DVI file, */
/* _until_ unescaped ESEQ_CHAR is found */
/* Return 1 if OK, 0 on error */
/****  dt2dv assumes 8 bit characters,      ****/
/****  but some day one might change that.  ****/
int set_seq(FILE* dtl, FILE* dvi) {
    int status = 1; /* status = 1 if OK, 0 if error */
    int more;       /* sequence of font characters continuing? */
    int escape = 0; /* flag set if previous character was an escape */
    int ch;         /* character read from DTL file */
    more = 1;
    while (more) {
        /* ignore read_char status, to allow unprintable characters */
        (void)read_char(dtl, &ch);
        /* but check for end of dtl file, or serious file reading error */
        if (ch < 0) {
            PRINT_PROGNAME;
            fprintf(stderr, "(set_seq) : ");
            fprintf(stderr, "end of dtl file, ");
            fprintf(stderr, "or serious dtl file reading error\n");
            dinfo();
            more = 0;
            status = 0; /* bad news */
        } else          /* read dtl file, okay */
        {
            if (ch == ESC_CHAR && escape == 0) /* escape next character */
            {
                escape = 1;
            } else {
                if (ch == ESEQ_CHAR && escape == 0) /* end of sequence */
                {
                    more = 0;
                } else if (ch <= 127) /* can use SETCHAR */
                {
                    put_byte(ch, dvi);
                } else if (ch <= 255) /* can use SET1 */
                {
                    put_byte(128, dvi); /* SET1 opcode */
                    put_unsigned(1, (U4)ch, dvi);
                } else {
                    PRINT_PROGNAME;
                    fprintf(stderr, "(set_seq) : ");
                    fprintf(
                        stderr,
                        "ERROR : DTL character %d is not in range 0 to 255\n",
                        ch);
                    dexit(1);
                    more = 0;
                    status =
                        0; /* Error, because dt2dv assumes 8 bit characters */
                }
                escape = 0; /* current ch is not an escape character */
            }
        }
    }
    return status;
}
/* set_seq */

/* translate unsigned n-byte hexadecimal number from dtl to dvi file. */
/* return value of hexadecimal number */
U4 xfer_hex(int n, FILE* dtl, FILE* dvi) {
    U4 unum = 0;             /* at most this space needed */
    COUNT nread = 0;         /* number of DTL bytes read by read_token */
    int nconv = 0;           /* number of arguments converted by sscanf */
    static Token token = ""; /* DTL token */

    if (n < 1 || n > 4) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(xfer_hex) : INTERNAL ERROR : asked for %d bytes.  Must be 1 "
                "to 4.\n",
                n);
        dexit(1);
    }

    nread = read_token(dtl, token);

    nconv = sscanf(token, HEX_FMT, &unum);

    if (nconv < 1) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_hex) : DTL FILE ERROR (%s) :  %s \"%s\".\n",
                dtl_filename, "hexadecimal number expected, not", token);
        dexit(1);
    }

    put_unsigned(n, unum, dvi);

    return unum;
}
/* xfer_hex */

/* translate unsigned n-byte octal number from dtl to dvi file. */
/* return value of octal number */
U4 xfer_oct(int n, FILE* dtl, FILE* dvi) {
    U4 unum = 0;             /* at most this space needed */
    COUNT nread = 0;         /* number of DTL bytes read by read_token */
    int nconv = 0;           /* number of arguments converted by sscanf */
    static Token token = ""; /* DTL token */

    if (n < 1 || n > 4) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(xfer_oct) : INTERNAL ERROR : asked for %d bytes.  Must be 1 "
                "to 4.\n",
                n);
        dexit(1);
    }

    nread = read_token(dtl, token);

    nconv = sscanf(token, OCT_FMT, &unum);

    if (nconv < 1) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_oct) : DTL FILE ERROR (%s) :  %s \"%s\".\n",
                dtl_filename, "octal number expected, not", token);
        dexit(1);
    }

    put_unsigned(n, unum, dvi);

    return unum;
}
/* xfer_oct */

/* translate unsigned n-byte number from dtl to dvi file. */
/* return value of unsigned number */
U4 xfer_unsigned(int n, FILE* dtl, FILE* dvi) {
    U4 unum = 0; /* at most this space needed */

    unum = get_unsigned(dtl);
    put_unsigned(n, unum, dvi);

    return unum;
}
/* xfer_unsigned */

/* translate signed n-byte number from dtl to dvi file. */
/* return value of signed number */
S4 xfer_signed(int n, FILE* dtl, FILE* dvi) {
    S4 snum = 0;

    snum = get_signed(dtl);
    put_signed(n, snum, dvi);

    return snum;
}
/* xfer_signed */

/* read unsigned number from dtl file. */
/* return value of unsigned number */
U4 get_unsigned(FILE* dtl) {
    U4 unum = 0;             /* at most this space needed */
    COUNT nread = 0;         /* number of DTL bytes read by read_token */
    int nconv = 0;           /* number of arguments converted by sscanf */
    static Token token = ""; /* DTL token */

    nread = read_token(dtl, token);

    nconv = sscanf(token, U4_FMT, &unum);

    if (nconv < 1) {
        PRINT_PROGNAME;
        fprintf(stderr, "(get_unsigned) : DTL FILE ERROR (%s) :  %s \"%s\".\n",
                dtl_filename, "unsigned number expected, not", token);
        dexit(1);
    }

    return unum;
}
/* get_unsigned */

/* read signed number from dtl file. */
/* return value of signed number */
S4 get_signed(FILE* dtl) {
    S4 snum = 0;
    COUNT nread = 0; /* number of DTL bytes read by read_token */
    int nconv = 0;   /* number of sscanf arguments converted and assigned */
    static Token token = "";

    nread = read_token(dtl, token);

    nconv = sscanf(token, S4_FMT, &snum);

    if (nconv < 1) {
        PRINT_PROGNAME;
        fprintf(stderr, "(get_signed) : DTL FILE ERROR (%s) :  %s \"%s\".\n",
                dtl_filename, "signed number expected, not", token);
        dexit(1);
    }

    return snum;
}
/* get_signed */

/* put unsigned in-byte integer to dvi file */
/* DVI format uses Big-endian storage of numbers: */
/* most significant byte is first. */
/* return number of bytes written. */
int put_unsigned(int n, U4 unum, FILE* dvi) {
    Byte ubyte[4]; /* at most 4 bytes needed in DVI format */
    int i;

    if (n < 1 || n > 4) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(put_unsigned) : INTERNAL ERROR : asked for %d bytes.  Must "
                "be 1 to 4.\n",
                n);
        dexit(1);
    }

    /* Big-endian storage. */
    for (i = 0; i < n; i++) {
        ubyte[i] = (Byte)(unum % 256);
        unum /= 256;
    }
    /* Reverse order for big-endian representation. */
    for (i = n - 1; i >= 0; i--) {
        put_byte((int)ubyte[i], dvi);
    }

    return n;
}
/* put_unsigned */

/* put signed in-byte integer to dvi file */
/* DVI format uses 2's complement Big-endian storage of signed numbers: */
/* most significant byte is first. */
/* return number of bytes written. */
int put_signed(int n, S4 snum, FILE* dvi) {
    /* Will this deal properly with the sign? */

    if (n < 1 || n > 4) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(put_signed) : INTERNAL ERROR : asked for %d bytes.  Must be "
                "1 to 4.\n",
                n);
        dexit(1);
    }

    /* How do we ensure 2's complement representation? */
    /* Here's a trick that I hope is portable, given ANSI C. */
    /* See K&R (2nd edition), Appendix A6.2 "Integral Conversions". */

    /* Convert snum to U4 data type */
    put_unsigned(n, (U4)snum, dvi);

    return n;
}
/* put_signed */

/* check that a BMES_CHAR is the next non-whitespace character in dtl */
int check_bmes(FILE* dtl) {
    int ch; /* next non-whitespace character in dtl */

    /* `(void)' because we ignore the number of spaces skipped */
    (void)skip_space(dtl, &ch);

    if (ch < 0) {
        PRINT_PROGNAME;
        fprintf(stderr, "(check_bmes) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "end of dtl file, or reading error\n");
        dexit(1);
    }

    if (ch != BMES_CHAR) {
        PRINT_PROGNAME;
        fprintf(stderr, "(check_bmes) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "BMES_CHAR (`%c') %s, not `%c' (char %d).\n", BMES_CHAR,
                "expected before string", ch, ch);
        dexit(1);
    }

    return 1; /* OK */
}
/* check_bmes */

/* check that an EMES_CHAR is the next character in dtl */
int check_emes(FILE* dtl) {
    int ch; /* dtl character */

    if (read_char(dtl, &ch) == 0) {
        PRINT_PROGNAME;
        fprintf(stderr, "(check_emes) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "end of dtl file, or reading error\n");
        dexit(1);
    }

    if (ch != EMES_CHAR) {
        PRINT_PROGNAME;
        fprintf(stderr, "(check_emes) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "EMES_CHAR (`%c') %s, not `%c' (char %d).\n", EMES_CHAR,
                "expected to follow string", ch, ch);
        dexit(1);
    }

    return 1; /* OK */
}
/* check_emes */


/** LString Fucntions.
 * 
 */

/// used in: xfer_len_string, fontdef.
void init_lstr(LStringPtr lsp, size_t n) {
    lsp->l = 0;
    lsp->m = n;
    lsp->s = (char*)gmalloc(n);
} /* init_lstr */

/// used in: xfer_len_string, fontdef.
void clear_lstr(LStringPtr lsp) {
    lsp->l = 0;
    lsp->m = 0;
    free(lsp->s);
    lsp->s = NULL; /* to be sure */
} /* clear_lstr */

/// [NOT_USED]
LStringPtr alloc_lstr(size_t n) {
    LStringPtr lsp;

    lsp = (LStringPtr)gmalloc(sizeof(LString));
    init_lstr(lsp, n);

    return lsp;
} /* alloc_lstr */

/// [NOT_USED]
void free_lstr(LStringPtr lsp) {
    clear_lstr(lsp);
    free(lsp);
} /* free_lstr */


/** write byte ch into LStringPtr lsp.
 *
 * used in: get_lstr
 */
void putch_lstr(int ch, LStringPtr lsp) {
    if (lsp->l >= lsp->m) {
        PRINT_PROGNAME;
        fprintf(stderr, "(putch_lstr) : ERROR : No more room in LString.\n");
        dexit(1);
    } else {
        lsp->s[(lsp->l)++] = ch;
    }
} /* putch_lstr */

/** get a string from dtl file, store as an LString in *lsp.
 *
 * lsp must already be allocated and initialised.
 * return length of LStringPtr lsp
 *
 * used in: xfer_len_string, fontdef.
 */
size_t get_lstr(FILE* dtl, LStringPtr lsp) {
    U4 nch;
    CharStatus char_status = CHAR_OK; /* OK so far */


    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(get_lstr) : entering get_lstr.\n");
    } /* if (debug) */

    check_bmes(dtl);

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(get_lstr) : string is: \"");
    } /* if (debug) */

    for (nch = 0; /***/; nch++) {
        int ch;

        char_status = read_string_char(dtl, &ch);
        if (char_status == CHAR_FAIL) {
            /* end of dtl file, or reading error */
            fprintf(stderr, "\n");
            PRINT_PROGNAME;
            fprintf(stderr,
                    "(get_lstr) : DTL FILE ERROR (%s) : ", dtl_filename);
            fprintf(stderr, "cannot read string[");
            fprintf(stderr, U4_FMT, nch);
            fprintf(stderr, "] from dtl file.\n");
            dexit(1);
        }

        if (debug) {
            fprintf(stderr, "%c", ch);
        } /* if (debug) */

        if (char_status == CHAR_EOS) {
            if (ch != EMES_CHAR) {
                PRINT_PROGNAME;
                fprintf(stderr, "(get_lstr) : INTERNAL ERROR : ");
                fprintf(stderr, "char_status = CHAR_FAIL,\n");
                fprintf(
                    stderr,
                    "but ch = %c (char %d) is not EMES_CHAR = %c (char %d)\n",
                    ch, ch, EMES_CHAR, EMES_CHAR);
                dexit(1);
            }
            (void)unread_char();
            break; /* end of string */
        } else if (char_status == CHAR_OK) {
            putch_lstr(ch, lsp);
        } else {
            PRINT_PROGNAME;
            fprintf(stderr, "(get_lstr) : INTERNAL ERROR : ");
            fprintf(stderr, "char_status = %d is unfamiliar!\n", char_status);
            dexit(1);
        } // end if (char_status <=>)
    } /* end for (nch = 0 ;; nch++) */

    if (debug) {
        fprintf(stderr, "\".\n");
    } /* if (debug) */

    check_emes(dtl);

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(get_lstr) : leaving get_lstr.\n");
    } /* if (debug) */

    return lsp->l;
} /* get_lstr */

/// used in: xfer_len_string, fontdef.
void put_lstr(LStringPtr lsp, FILE* dvi) {
    size_t fw_ret;

    fw_ret = fwrite(lsp->s, sizeof(char), lsp->l, dvi);
    dvi_written += fw_ret;

    if (fw_ret < lsp->l) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(put_lstr) : DVI File ERROR : not all bytes written ");
        fprintf(stderr, "(%zd of %zd).\n", fw_ret, lsp->l);
        dexit(1);
    }
} /* put_lstr */

/** transfer (length and) quoted string from dtl to dvi file,
 * return number of bytes written to dvi file.
 */
U4 xfer_len_string(int n, FILE* dtl, FILE* dvi) {
    U4 k, k2;
    LString lstr;

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_len_string) : entering xfer_len_string.\n");
    } /* if (debug) */

    init_lstr(&lstr, LSTR_SIZE);

    /* k[n] : length of special string */
    k = get_unsigned(dtl);
    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_len_string) : string's nominal length k = ");
        fprintf(stderr, U4_FMT, k);
        fprintf(stderr, " characters.\n");
    } /* if (debug) */

    k2 = get_lstr(dtl, &lstr);
    if (k2 != k) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_len_string) : WARNING : string length (");
        fprintf(stderr, U4_FMT, k);
        fprintf(stderr, ") in DTL file is wrong\n");
        fprintf(stderr, "Writing correct value (");
        fprintf(stderr, U4_FMT, k2);
        fprintf(stderr, ") to DVI file\n");
    }

    put_unsigned(n, k2, dvi);
    put_lstr(&lstr, dvi);

    clear_lstr(&lstr);
    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_len_string) : leaving xfer_len_string.\n");
    } /* if (debug) */

    return (n + k2);
} /* xfer_len_string */

/* translate signed 4-byte bop address from dtl to dvi file. */
/* return value of bop address written to DVI file */
S4 xfer_bop_address(FILE* dtl, FILE* dvi) {
    S4 snum = 0;             /* at most this space needed for byte address */
    COUNT nread = 0;         /* number of DTL bytes read by read_token */
    int nconv = 0;           /* number of arguments converted by sscanf */
    static Token token = ""; /* DTL token */

    nread += read_token(dtl, token);

    nconv = sscanf(token, S4_FMT, &snum);

    if (nconv != 1) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(xfer_bop_address) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "signed number expected, not \"%s\".\n", token);
        dexit(1);
    }

    if (snum != last_bop_address) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_bop_address) : WARNING : byte address (");
        fprintf(stderr, S4_FMT, snum);
        fprintf(stderr, ")\n");
        fprintf(stderr, "for previous bop in DTL file is wrong\n");
        fprintf(stderr, "Writing correct value (");
        fprintf(stderr, WORD_FMT, last_bop_address);
        fprintf(stderr, ") to DVI file\n");
    }

    put_signed(4, last_bop_address, dvi);

    return last_bop_address;
}
/* xfer_bop_address */

/* translate signed 4-byte postamble address from dtl to dvi file. */
/* return value of postamble address written to DVI file */
S4 xfer_postamble_address(FILE* dtl, FILE* dvi) {
    S4 snum = 0;             /* at most this space needed for byte address */
    COUNT nread = 0;         /* number of DTL bytes read by read_token */
    int nconv = 0;           /* number of arguments converted by sscanf */
    static Token token = ""; /* DTL token */

    nread += read_token(dtl, token);

    nconv = sscanf(token, S4_FMT, &snum);

    if (nconv != 1) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_postamble_address) : DTL FILE ERROR (%s) : ",
                dtl_filename);
        fprintf(stderr, "signed number expected, not \"%s\".\n", token);
        dexit(1);
    }

    if (snum != postamble_address) {
        PRINT_PROGNAME;
        fprintf(stderr, "(xfer_postamble_address) : WARNING : byte address (");
        fprintf(stderr, S4_FMT, snum);
        fprintf(stderr, ")\n");
        fprintf(stderr, "for postamble in DTL file is wrong\n");
        fprintf(stderr, "Writing correct value (");
        fprintf(stderr, WORD_FMT, postamble_address);
        fprintf(stderr, ") to DVI file\n");
    }

    put_signed(4, postamble_address, dvi);

    return postamble_address;
}
/* xfer_postamble_address */

int put_table(op_table table, int opcode, FILE* dtl, FILE* dvi) {
    /* table:  {char * name; int first, last; op_info * list; }; */
    /* op_info:   {int code; char * name; int nargs; char * args; }; */

    op_info op; /* entry in table */
    int i;
    int pos; /* current position in string being scanned */
    static String args = "";

    /* Defensive programming. */
    if (opcode < table.first || opcode > table.last) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(put_table) : DTL FILE (OR INTERNAL) ERROR : opcode %d ",
                opcode);
        fprintf(stderr, "is outside table %s [ %d to %d ] !\n", table.name,
                table.first, table.last);
        dexit(1);
    }

    op = table.list[opcode - table.first];

    /* More defensive programming. */
    if (opcode != op.code) {
        PRINT_PROGNAME;
        fprintf(stderr,
                "(put_table) : INTERNAL ERROR : opcode %d for command \"%s\"",
                opcode, op.name);
        fprintf(stderr, " faulty in table \"%s\".\n", table.name);
        dexit(1);
    }

    /* process all the arguments, according to size and sign */

    strncpy(args, op.args, MAXSTRLEN);

    pos = 0;
    for (i = 0; i < op.nargs; i++) {
        int argtype = 0;
        int nscan = 0; /* number of bytes read by sscanf */
        int nconv = 0; /* number of sscanf arguments converted & assigned */

        /* sscanf() does NOT advance over its input: */
        /* C strings lack internal state information, which C files have. */
        /* On Sun/OS, sscanf calls ungetc on the string it reads, */
        /* which therefore has to be writable. */

        nconv = sscanf(args + pos, "%d%n", &argtype, &nscan);

        if (nconv < 1 || nscan < 1) {
            PRINT_PROGNAME;
            fprintf(stderr,
                    "(put_table) : INTERNAL ERROR : internal read of table %s "
                    "failed!\n",
                    table.name);
            dexit(1);
        }

        pos += nscan;

        if (argtype < 0)
            xfer_signed(-argtype, dtl, dvi);
        else
            xfer_unsigned(argtype, dtl, dvi);
    }
    /* end for */

    return 1; /* OK */
}
/* put_table */

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* read special (1 <= n <= 4 byte) data from dtl, and write in dvi */
/* return number of bytes written */
U4 special(FILE* dtl, FILE* dvi, int n) {
    U4 nk;

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(special) : entering special.\n");
    }

    if (n < 1 || n > 4) {
        PRINT_PROGNAME;
        fprintf(stderr, "(special) : DTL FILE ERROR (%s) : special %d, ",
                dtl_filename, n);
        fprintf(stderr, "range is 1 to 4.\n");
        dexit(1);
    }

    /* k[n] : length of special string */
    /* x[k] : special string */
    /* nk = n + k */
    nk = xfer_len_string(n, dtl, dvi);

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(special) : leaving special.\n");
    }

    return (nk);
}
/* special */

/* read fontdef fnt_def1 .. fnt_def4 from dtl, and write in dvi */
/* suffix is the fontdef suffix : 1 to 4 */
/* return number of bytes written */
int fontdef(FILE* dtl, FILE* dvi, int suffix) {
    U4 a, l, a2, l2;
    U4 k;
    LString lstr1, lstr2;

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(fontdef) : entering fontdef.\n");
    }

    if (suffix < 1 || suffix > 4) {
        PRINT_PROGNAME;
        fprintf(stderr, "(fontdef) : DTL FILE ERROR (%s) : ", dtl_filename);
        fprintf(stderr, "font def %d, but range is 1 to 4.\n", suffix);
        dexit(1);
    }

    init_lstr(&lstr1, LSTR_SIZE);
    init_lstr(&lstr2, LSTR_SIZE);

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(fontdef) : about to read font number.\n");
    }

    /* k[suffix] : font number */
    if (suffix == 4)
        k = xfer_signed(suffix, dtl, dvi);
    else
        k = xfer_unsigned(suffix, dtl, dvi);

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(fontdef) : font ");
        fprintf(stderr, U4_FMT, k);
        fprintf(stderr, ".\n");
    }

#ifdef HEX_CHECKSUM
    /* c[4] : (hexadecimal) checksum : I (gt) would prefer this */
    xfer_hex(4, dtl, dvi);
#else /* NOT HEX_CHECKSUM */
    /* c[4] : checksum (octal, for comparison with tftopl's .pl file) */
    xfer_oct(4, dtl, dvi);
#endif

    /* s[4] */
    xfer_unsigned(4, dtl, dvi);

    /* d[4] */
    xfer_unsigned(4, dtl, dvi);

    /* If DTL file's edited, a and l may be wrong. */

    /* a[1] : length of font `area' (directory) portion of pathname string */
    a = get_unsigned(dtl);

    /* l[1] : length of font portion of pathname string */
    l = get_unsigned(dtl);

    /* n[a+l] : font pathname string <= area + font */
    a2 = get_lstr(dtl, &lstr1);
    if (a2 != a) {
        PRINT_PROGNAME;
        fprintf(stderr, "(fontdef) : WARNING : font area string's length (");
        fprintf(stderr, U4_FMT, a);
        fprintf(stderr, ") in DTL file is wrong\n");
        fprintf(stderr, "Writing correct value (");
        fprintf(stderr, U4_FMT, a2);
        fprintf(stderr, ") to DVI file\n");
    }

    put_unsigned(1, a2, dvi);

    l2 = get_lstr(dtl, &lstr2);
    if (l2 != l) {
        PRINT_PROGNAME;
        fprintf(stderr, "(fontdef) : WARNING : font string's length (");
        fprintf(stderr, U4_FMT, l);
        fprintf(stderr, ") in DTL file is wrong\n");
        fprintf(stderr, "Writing correct value (");
        fprintf(stderr, U4_FMT, l2);
        fprintf(stderr, ") to DVI file\n");
    }

    put_unsigned(1, l2, dvi);

    put_lstr(&lstr1, dvi);
    put_lstr(&lstr2, dvi);

    clear_lstr(&lstr1);
    clear_lstr(&lstr2);

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(fontdef) : leaving fontdef.\n");
    }

    return (suffix + 4 * 4 + 2 * 1 + a2 + l2);
} /* fontdef */


/* read preamble from dtl, and write in dvi */
/* return number of bytes written */
U4 preamble(FILE* dtl, FILE* dvi) {
    U4 k1;

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(preamble) : entering preamble.\n");
    }

    /* i[1] */
    xfer_unsigned(1, dtl, dvi);

    /* num[4] */
    xfer_unsigned(4, dtl, dvi);

    /* den[4] */
    xfer_unsigned(4, dtl, dvi);

    /* mag[4] */
    xfer_unsigned(4, dtl, dvi);

    /* k[1] : length of comment */
    /* x[k] : comment string */
    /* k1 = 1 + k */
    k1 = xfer_len_string(1, dtl, dvi);

    if (debug) {
        PRINT_PROGNAME;
        fprintf(stderr, "(preamble) : leaving preamble.\n");
    }

    return (1 + 3 * 4 + k1);
}
/* preamble */

/* read postamble from dtl, and write in dvi */
/* return number of bytes written */
int postamble(FILE* dtl, FILE* dvi) {
    postamble_address = dvi_written - 1;

    /* p[4] : DVI address of previous bop command */
    /*        --- unsigned? --- or signed, as I assume? */
    /* For, surely  p  should be  -1  if the DVI file has NO bop? */
    xfer_bop_address(dtl, dvi);

    /* num[4] */
    xfer_unsigned(4, dtl, dvi);

    /* den[4] */
    xfer_unsigned(4, dtl, dvi);

    /* mag[4] */
    xfer_unsigned(4, dtl, dvi);

    /* l[4] */
    xfer_unsigned(4, dtl, dvi);

    /* u[4] */
    xfer_unsigned(4, dtl, dvi);

    /* s[2] */
    xfer_unsigned(2, dtl, dvi);

    /* t[2] */
    xfer_unsigned(2, dtl, dvi);

    return (6 * 4 + 2 * 2);
}
/* postamble */

/* read post_post from dtl, and write in dvi */
/* return number of bytes written */
int post_post(FILE* dtl, FILE* dvi) {
    /* hope I'm writing the "223" bytes in an 8-bit clean way */
    int n223 = 0; /* number of "223" bytes in final padding */

    /* q[4] : DVI address of post command */
    /*        --- unsigned? --- or signed, as I assume? */
    /* what happens if there is NO postamble command? */
    /* shouldn't  q  be  -1  then? */

    xfer_postamble_address(dtl, dvi);

    /* i[1] : DVI identification byte = 2 */
    xfer_unsigned(1, dtl, dvi);

    for (n223 = 0; true; n223++) {
        COUNT nread = 0; /* number of DTL bytes read by read_token */
        static Token token;

        strcpy(token, "");

        nread = read_token(dtl, token);

        /* check whether end of dtl file */
        if (nread == 0) {
            if (group) {
                /* dtl file shouldn't end before an ECOM */
                PRINT_PROGNAME;
                fprintf(stderr,
                        "(post_post) : DTL FILE ERROR (%s) : ", dtl_filename);
                fprintf(stderr, "premature end of DTL file!\n");
                fprintf(stderr,
                        "%d complete iterations of \"padding byte\" loop;\n",
                        n223);
                fprintf(stderr, "troublesome token = \"%s\"\n", token);
                dexit(1);
            }
            /* leave the "223" loop */
            break;
        } else if (strcmp(token, "223") == 0) {
            /* token is a "223" padding byte */
            /* loop again */
        } else {
            /* read a non-empty token that wasn't "223" */
            (void)unread_char();
            if (group) {
                if (strcmp(token, ECOM) == 0) {
                    /* end of DTL's post_post command */
                } else {
                    /* error : expected end of post_post */
                    PRINT_PROGNAME;
                    fprintf(stderr, "(post_post) : DTL FILE ERROR (%s) : ",
                            dtl_filename);
                    fprintf(stderr, "token \"%s\" should be ECOM (\"%s\")\n",
                            token, ECOM);
                    dexit(1);
                }
            }
            /* leave the "223" loop */
            break;
        }
    }
    /* end for */

    if (n223 < 4) {
        PRINT_PROGNAME;
        fprintf(stderr, "(post_post) : DTL FILE ERROR (%s) : \n", dtl_filename);
        fprintf(stderr, "fewer than four `223' padding bytes.\n");
        fprintf(stderr, "Will write at least four `223' padding bytes.\n");
    }

    /* check whether the DVI file size is a multiple of 4 bytes */
    if ((dvi_written + n223) % 4 != 0) {
        PRINT_PROGNAME;
        fprintf(stderr, "(post_post) : WARNING : \n");
        fprintf(stderr, "DVI size ");
        fprintf(stderr, COUNT_FMT, dvi_written);
        fprintf(stderr, " (bytes) wouldn't be a multiple of 4 !\n");
        fprintf(
            stderr,
            "Will write (at least four) `223' padding bytes until it is.\n");
    }

    /* final padding of DVI file by "223" bytes to a multiple of 4 bytes, */
    /* with at least 4 bytes */

    for (n223 = 0; (n223 < 4) || (dvi_written % 4 != 0); n223++) {
        /* add a "223" padding byte */
        put_byte(223, dvi);
    }

    return (4 + 1 + n223);
}
/* post_post */

/* end of dt2dv.c */