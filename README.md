[![Build Status](https://travis-ci.org/TeX-host/dtl.svg?branch=master)](https://travis-ci.org/TeX-host/dtl)
[![codecov](https://codecov.io/gh/TeX-host/dtl/branch/master/graph/badge.svg)](https://codecov.io/gh/TeX-host/dtl)
[![License: CC0-1.0](https://img.shields.io/badge/License-CC0%201.0-lightgrey.svg)](http://creativecommons.org/publicdomain/zero/1.0/)

> This file is public domain.
> 
> Originally written 1995, Geoffrey Tobin.
> 
> The author has expressed the hope that any modification will retain enough content to remain useful. 
> He would also appreciate being acknowledged as the original author in the documentation.
> 
> This declaration added 2008/11/14 by Clea F. Rees with the permission of Geoffrey Tobin.

README for DTL package - Thu 9 March 1995
-----------------------------------------

+ Author: Geoffrey Tobin <G.Tobin@ee.latrobe.edu.au>
+ Version: 0.6.1
+ CTAN Archive-path: [dviware/dtl](https://ctan.org/pkg/dtl)
+ Brief Description:  
DTL (DVI Text Language) files are equivalent to TeX's DVI files, 
but are humanly readable, instead of binary.  Two programs are
provided to translate between DVI and DTL: `dv2dt`, `dt2dv`.
`dt2dv` warns if byte addresses or string lengths recorded in a DTL
file are incorrect, then overrides them.  This makes DTL files
editable.  It also allows quoted apostrophes (`\'`) and quoted quotes
(`\\`) in strings.  The current DTL variety, sequences-6, separates
font paths into directory and font, which makes them freely editable.
In this release, DTL line numbers are correctly calculated, and three
memory leaks have been fixed.
+ Keywords: dvi, TeX
+ Includes:
 Makefile  README  dt2dv.c  dtl.h  dv2dt.c
 man2ps  dtl.doc  dvi.doc  dt2dv.man  dv2dt.man
 hello.tex  example.tex  tripvdu.tex  edited.txt

## Motivation:

 When TeX has typeset a document, it writes its handiwork to a DVI
file, for DVI processing software (such as viewers, printer drivers,
dvidvi, and dvicopy) to read.

The file  dvi.doc  lists the DVI file commands, with their opcodes
(byte values), nominal command names, arguments, and meanings.  For a
detailed description of DVI file structure, see one of these:
1.  Donald E. Knuth's book _TeX: The Program_;
2.  The file tex.web, which contains source and documentation for TeX:  
        CTAN:  [`systems/knuth/tex/tex.web`][tex.web]
3.  The source for Knuth's dvitype program:  
        CTAN:  [`systems/knuth/texware/dvitype.web`][dvitype.web]
4.  Joachim Schrod's DVI drivers standard document, the relevant part
    of which is at  
        CTAN:  [`dviware/driv-standard/level-0`][dvistdlv0]

[tex.web]: https://ctan.org/tex-archive/systems/knuth/dist/tex
[dvitype.web]: https://ctan.org/tex-archive/systems/knuth/dist/texware
[dvistdlv0]: https://www.ctan.org/tex-archive/dviware/driv-standard/level-0

Sometimes human beings are interested to see exactly what TeX has
produced, for example when viewing or printing of the DVI file gives
unexpected results.  However, a DVI file is a compact binary
representation, so we need software to display its contents.

Binary file editors, when available, can show the DVI bytes, but not
their meanings, except for the portions that represent embedded text.
In particular, the command names are not shown, and the command
boundaries are not respected.

By contrast, Knuth's dvitype program is designed as an example of a
DVI driver.  However, dvitype is inconvenient for studying the DVI
file alone, for the following reasons:
1.  Being a DVI driver, dvitype endeavors to read the TFM font metric
files referenced in the DVI file.  If a TFM file is absent, dvitype
quits with an error message.
2.  When it starts, it prompts the user interactively for each of a
series of options.
3.  Even the least verbose option gives masses of information that is
not contained in the DVI file, coming instead from a combination of
the data in the DVI file and TFM files.
4.  It does NOT show the DVI information in a way that accurately
reflects the structure of the DVI file.
5.  Its output, if redirected to a file, produces a very large file.
6.  There is no automated procedure for converting the output of
dvitype back to a DVI file, and doing it by hand is totally
unreasonable.

The first disadvantage is a killer if a TFM file is absent.
Disadvantages two to four make dvitype very inconvenient for studying
a DVI file.  The fifth problem makes dvitype's output tedious,
disk-hungry (so one deletes it almost immediately), and unsuitable for
file transfer.

The sixth disadvantage of dvitype is important to those people who are
interested in editing DVI files.  Since the DVI files refer explicitly
to their own internal byte addresses, it's very easy to mess up a DVI
file if one were to try to edit it directly, even apart from the problem
of how to recognise a command.

So an exact, concise, textual representation of a DVI file is needed,
but dvitype does not produce one.

## Resolution:

 Therefore, working from Joachim Schrod's description, I designed DTL
and its conversion programs dv2dt (DVI -> DTL) and dt2dv (DTL -> DVI),
which are provided as C sources:

    dtl.h
    dv2dt.c
    dt2dv.c

Although I was motivated by the TFM <-> PL conversion provided by
Knuth's tftopl and pltotf programs, I deliberately designed DTL to be
a much more concise and literal translation than the `property list'
structure exemplified by PL.  The result is that a DTL file is
typically three times the size of its equivalent DVI file.  The
document  dtl.doc  lists the correspondence between the DTL command
names and the (nominal) DVI command names.

A clear advantage of an exact two-way conversion is that we can check
(and prove) whether the converters worked truly on a given DVI file.
The provided plain TeX files:

    example.tex
    tripvdu.tex

can be used to test whether the compiled programs are behaving
sensibly.  Whereas  example.tex  is a simple document that uses a
variety of plain TeX commands,  tripvdu.tex  provides a kind of
`trip test` for DVI processor programs.  Both documents are taken,
with permission, from Andrew K. Trevorrow's dvitovdu (alias dvi2vdu)
distribution (and are also part of the dvgt viewer distribution).

The  Makefile  might have to be edited for your site, as it assumes
gcc  for your C compiler.  Makefile compiles dv2dt and dt2dv, then
runs  tex  on example.tex and tripvdu.tex, and also converts the
resulting DVI files to DTL files, back to DVI files (with a change
of name), then back again to DTL files, so that the results can be
compared using a textual differencing program.  (Many computer systems
have such a program; on unix, as assumed by Makefile, this is named
`diff`;  ms-dos has one named `comp`.)  This should produce a
zero-length  .dif  file for each document, proving that the two DTL
files are identical.

A keen tester might also use a binary difference program on the DVI
files, to check that they are identical, as they need to be.  (On unix
systems, the `diff` program suffices for that purpose.)

## Note:

 In representing numeric quantities, I have mainly opted to use
decimal notation, as this is how most of us are trained to think.
However, for the checksums in the `fd` (font definition) commands, I
chose octal notation, as this is used for checksums in Knuth's PL
files, against which DVI files must be compared when a DVI driver
loads a font.

## Caveat:

The length of DTL commands is limited by the size of the line buffer
in `dt2dv.c`.

End of README
