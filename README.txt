    README for package 'moot'

    Last updated for moot version 2.0.7

DESCRIPTION
    moot - moocow's part-of-speech tagger and utilities.

REQUIREMENTS
    pkg-config (Required)
        Available from: http://www.freedesktop.org/software/pkgconfig/

        To build from cvs, you will also need the pkg-config autoconf macros
        which come with the source distribution of pkg-config.

    STL headers (Required)
        If your C++ compiler does not have the STL headers already
        installed, you will need to get them from somewhere. For gcc-2.x, I
        recommend STLport >= 4.5.3, available from http://www.stlport.org
        You will have to set the environment variables CPPFLAGS, LDFLAGS,
        and LIBS according to your installation before building. Newer gcc
        versions (3.x,4.x) have native STL support.

    flex++ , bison++ (Optional)
        Alain Coetmeur's C++ ports of the famous lexer/parser generator
        pair, available from: ftp://iecc.com/pub/file/bison++flex++ or from
        the official distribution site of this package.

        Tested with flex++-v2.3.8-4 and bison++-v1.21-5.

        Should only be required if you want/need to mess with the native I/O
        formats. If the build fails during 'make', just run:

         touch src/libmoot/*Lexer.*[ch] src/libmoot/*Parser.*[ch]

        from the distribution root directory, and call make again. Better
        yet, calling ./configure with:

         ./configure FLEXXX=no BISONXX=no

        ought to do the trick too.

    re2c (Optional)
        Tested with re2c v0.16.

        Scanner generator used for compiling some WASTE library sources.
        re2c versions later than v0.16 have been observed to cause segaults.
        If this heppens to you, you can revert revert the original source
        files from the distribution or version control, e.g.

         for f in src/libmoot/*.re* ; do
           svn revert ${f%.re*}.{cc,h} && touch ${f%.re*}.{cc,h}
         done

        Better yet, call ./configure with:

         ./configure RE2C=no

    expat (Optional)
        XML parser toolkit library by James Clark, required for XML input,
        available from http://expat.sourceforge.net

        Tested version(s): 1.95.6, 1.95.8.

    librecode (Optional)
        Character-set recoding library by François Pinard, useful for XML
        output, available from http://www.gnu.org/directory/recode.html

        Tested version(s): 3.6.

    zlib (Optional)
        Compression library by Jean-loup Gailly and Mark Adler, useful for
        compressed binary HMM files. Available from:
        http://www.gzip.org/zlib

        Tested version(s): 1.2.1., 1.2.3.3

    doxygen (Optional)
        Required for building library documentation. Available from:
        http://www.doxygen.org

        Tested version(s): 1.2.15, 1.5.4

    Perl (Optional)
        Get it from http://www.cpan.org or http://www.perl.com Required for
        building command-line parsers, utility documentation, library
        documentation, etc.

    Getopt::Gen (Optional)
        A Perl module used to generate command-line option parsers.
        Available from:
        http://www.ling.uni-potsdam.de/~moocow/projects/perl/index.html#gog

        If make fails due to lacking optgen.perl, either get & install this
        module, or run:

         touch src/programs/*_cmdparser.*[ch]

        from the distribution root directory, and call make again.

        Tested version(s): 0.13

    pod2man, pod2text, pod2html, ... (Optional)
        The Perl documentation converstion utilities, required for
        (re-)building the correspdonding program documentation formats.
        These should have come with your Perl. On Debian derivates, they
        live in the "perl-doc" package. They should be automatically
        detected by ./configure if they are installed in your PATH.
        Otherwise, you can use the variables POD2MAN, POD2TEXT; POD2HTML,
        and/or POD2LATEX to specify their locations.

        Manpages and HTML documentation are distributed with the "official"
        source distribution, so you should only need the "pod2xyz" programs
        if you're building from CVS/SVN, or if you want to generate one or
        more additional documentation formats (e.g. text, DVI, PostScript,
        or PDF).

INSTALLATION
    Issue the following commands to the shell:

     sh ./configure
     make
     make install

    See the file INSTALL in the top-level distribution directory for
    details.

BUILD FROM CVS/SVN
    To build from CVS or SVN sources, you need the GNU utilities aclocal,
    automake, autoconf, and libtool. If you have these, you can just run the
    top-level script:

     sh ./autoreconf.sh

    This will create the 'configure' script and other necessary build files.

    You might also need Perl and the Getopt::Gen Perl module, which should
    be available from wherever you acquired these sources.

SYSTEMS ON WHICH MOOT HAS SUCCESSFULLY COMPILED
    linux / gcc
    MacOS-X / gcc
    win32 / mingw32
    ... more to come, maybe...

KNOWN ISSUES
  Common Warnings
    "WARNING: ..." from configure
        This is a warning. This is only a warning. It may be responsible for
        fatal errors from 'make' if and only if you are building from SVN.
        Otherwise, you can safely ignore it (probably).

    "flex++bison++.pc not found"
        If you want this to go away, install my (old, unmaintained)
        flex++bison++ package from:

         http://www.ling.uni-potsdam.de/~moocow/projects/moot/flex++bison++-0.0.5.tar.gz

        Otherwise, keep your distro's versions and ignore the warning.

    "cannot find optgen.pl program"
        If you're building from CVS/SVN, this will be fatal. Get my
        Getopt::Gen perl module (and perl, if you haven't already), build
        it, install it, then run moot's ./configure again.

  Known Bugs
    "Unknown 'strict' tag(s) '1' at ... Parse/Template.pm line 2"
        Your Parse::Lex module is broken. It turns out that Parse::Lex v2.15
        is Just Plain Broken, and it appears to be unmaintained. Get the
        sources from CPAN, and comment out all of the offending 'use strict
        ...' lines, or just install the hacked version from:

         http://www.ling.uni-potsdam.de/~moocow/projects/perl/ParseLex-2.15-hacked.tar.gz

    "osfcn.h No such file or directory"
        osfcn.h appears to be a relic of my antiquated flex++bison++
        package; you may attempt to use and/or modify the file
        'src/libmoot/myosfcn.h' that comes with the distribution as a
        workaround. Otherwise, you should try to rebuild the lexer/parser
        .cc and .h files by hand:

          bash$ cd moot-XX.YY
          bash$ touch ./src/libmoot/*.ll ./src/libmoot/*.yy
          bash$ make

        ... ought to do the trick, assuming you have Coetmeur's flex++ and
        bison++ installed.

ACKNOWLEDGEMENTS
    Development of this package was supported by the project 'Kollokationen
    im Wörterbuch' ("collocations in the dictionary",
    http://www.bbaw.de/forschung/kollokationen ) in association with the
    project 'Digitales Wörterbuch deutscher Sprache des 20. Jahrhunderts
    (DWDS)' ("digital dictionary of the German language of the 20th
    century", http://www.dwds.de ) at the Berlin-Brandenburgische Akademie
    der Wissenschaften ( http://www.bbaw.de ) with funding from the
    Alexander von Humboldt Stiftung ( http://www.avh.de ) and from the
    Zukunftsinvestitionsprogramm of the German federal government.

    I am grateful to Christiane Fellbaum, Alexander Geyken, Thomas
    Hanneforth, Gerald Neumann, Edmund Pohl, Alexey Sokirko, and others for
    offering useful insights in the course of development of this package.

    Thomas Hanneforth wrote and maintains the libFSM C++ library for
    finite-state device operations used in the development of the HMM tagger
    / disambiguator.

    Alexander Geyken and Thomas Hanneforth developed the rule-based
    morphological analysis system for German which was used in the
    development and testing of the class-based HMM tagger / disambiguator.

AUTHOR
    Bryan Jurish <moocow@cpan.org>

