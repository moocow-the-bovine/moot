/*--------------------------------------------------------------------------
 * File: dwdspp_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : main()
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dwdspp_cmdparser.h"
//#include "dwdspp.h"
#include "dwdspp_lexer.h"

using namespace std;

extern "C" int getline(char **, size_t *, FILE *);

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdspp";

// options
gengetopt_args_info args;

// filenames
char *infile = "-";
char *outfile = "-";
char *inlistfile = "-";

// files
FILE *in = NULL;
FILE *out = NULL;
FILE *inlist = NULL;

// for file-lists
char *linebuf = NULL;
size_t lbsize = 0;

// for argument-stepping
unsigned int input_num = 0;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- show banner
  if (args.verbose_arg > 0)
    fprintf(stderr,
	    "\n%s version %s by Bryan Jurish <jurish@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  // -- output file
  if (strcmp(args.output_arg,"-") != 0) {
    out = fopen(args.output_arg, "w");
  } else {
    out = stdout;
  }
  if (!out) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n", PROGNAME, args.output_arg, strerror(errno));
    exit(1);
  }
}

/*--------------------------------------------------------------------------
 * file-stepping
 *--------------------------------------------------------------------------*/

/*
 * inlist = step_inlist_file();
 *  - sets globals "FILE *inlist" and "char *inlistfile" as side effect
 *  - returns global "FILE *inlist"
 *  - returns NULL if no more inlistfiles are available
 */
FILE *next_inlist_file() {
    if (inlist && inlist != stdin) fclose(inlist);
    if (!args.list_given) return NULL;
    if (++input_num >= args.inputs_num) return NULL;

    inlistfile = args.inputs[input_num];
    inlist = fopen(args.inputs[input_num], "r");
    if (!inlist) {
	fprintf(stderr,"%s: open failed for input list-file '%s': %s\n",
		PROGNAME, inlistfile, strerror(errno));
	exit(1);
    }
    return inlist;
}

/*
 * in = first_file();
 *  - sets globals "FILE *in" and "char *infile" as side effect
 *  - returns global "FILE *in"
 */
FILE *first_file() {
    if (args.list_given) {
	// -- args/inputs are file-LISTS
	if (args.inputs_num == 0) {
	    // -- file-list on stdin
	    inlistfile = "<stdin>";
	    inlist = stdin;
	} else {
	    // -- file-list(s) given
	    inlistfile = args.inputs[0];
	    inlist = fopen(args.inputs[0], "r");
	}
	// -- sanity check
	if (!inlist) {
	    fprintf(stderr,"%s: open failed for input list-file '%s': %s\n",
		    PROGNAME, inlistfile, strerror(errno));
	    exit(1);
	}
	// -- read next file
	while (getline(&linebuf, &lbsize, inlist) == -1) {
	    if (feof(inlist)) {
		if (!(inlist = next_inlist_file())) return NULL;
	    } else {
		fprintf(stderr, "%s: Error reading input-list-file '%s': %s\n",
			PROGNAME, inlistfile, strerror(errno));
		exit(1);
	    }
	}
	infile = linebuf;
	infile[strlen(infile)-1] = '\0';  // eliminate trailing newline
	if (!(in = fopen(infile, "r"))) {
	    fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
		    PROGNAME, infile, strerror(errno));
	    exit(1);
	}
	return in;
    }

    // -- args/inputs are file-NAMES
    if (args.inputs_num == 0) {
	// -- read from stdin
	infile = "<stdin>";
	in = stdin;
    } else {
	// -- read from files
	infile = args.inputs[0];
	if (!(in = fopen(infile, "r"))) {
	    fprintf(stderr, "%s: open failed for input-file '%s': %s",
		    PROGNAME, infile, strerror(errno));
	    exit(1);
	}
    }
    return in;
}


/*
 * in = next_file();
 *  - sets globals "FILE *in" and "char *infile" as side effect
 *  - returns global "FILE *in"
 */
FILE *next_file() {
    if (in && in != stdin) fclose(in);

    if (args.list_given) {
	// -- read next file
	while (getline(&linebuf, &lbsize, inlist) == -1) {
	    if (feof(inlist)) {
		if (!(inlist = next_inlist_file())) return NULL;
	    } else {
		fprintf(stderr, "%s: Error reading input-list-file '%s': %s\n",
			PROGNAME, inlistfile, strerror(errno));
		exit(1);
	    }
	}
	infile = linebuf;
	infile[strlen(infile)-1] = '\0';  // eliminate trailing newline
	if (!(in = fopen(infile, "r"))) {
	    fprintf(stderr, "%s: open failed for input-file '%s': %s\n",
		    PROGNAME, infile, strerror(errno));
	    exit(1);
	}
	return in;
    }

    // -- args/inputs are file-NAMES
    if (++input_num >= args.inputs_num) return NULL;

    infile = args.inputs[input_num];
    if (!(in = fopen(infile, "r"))) {
	fprintf(stderr, "%s: open failed for input-file '%s': %s",
		PROGNAME, infile, strerror(errno));
	exit(1);
    }
    return in;
}


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  dwdspp_lexer lexer;
  int numfiles = 0;
  timeval started,stopped;
  double elapsed;

  GetMyOptions(argc,argv);
  lexer.verbose = args.verbose_arg;

  // -- get start time
  if (args.verbose_arg > 0) {
      gettimeofday(&started, NULL);
  }

  // -- big loop
  for (in = first_file(); in; in = next_file()) {
      if (args.verbose_arg > 0) {
	numfiles++;
	if (args.verbose_arg > 1) {
	    fprintf(stderr,"%s: processing file '%s'... ", PROGNAME, infile);
	    fflush(stderr);
	}
      }
      fprintf(out, "\n# %s: File: %s\n\n", PROGNAME, infile);

      lexer.tokenize_stream(in,out);

      if (args.verbose_arg > 1) {
	fprintf(stderr," done.\n");
	fflush(stderr);
      }
  }

  // -- summary
  if (args.verbose_arg > 0) {
      // -- timing
      gettimeofday(&stopped, NULL);
      elapsed = stopped.tv_sec-started.tv_sec + (stopped.tv_usec-started.tv_usec)/1000000.0;

      // -- print summary
      fprintf(stderr, "\n-----------------------------------------------------\n");
      fprintf(stderr, "%s Summary:\n", PROGNAME);
      fprintf(stderr, "  + Files processed : %d\n", numfiles);
      fprintf(stderr, "  + Tokens found    : %d\n", lexer.numwords);
      fprintf(stderr, "  + Time Elsapsed   : %.2f sec\n", elapsed);
      fprintf(stderr, "  + Throughput      : %.2f toks/sec\n", (float)lexer.numwords/elapsed);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}

