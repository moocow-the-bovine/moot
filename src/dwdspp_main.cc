/*--------------------------------------------------------------------------
 * File: dwdspp_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : main()
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dwdspp_cmdparser.h"
#include "dwdspp.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdspp";

// options
gengetopt_args_info args;

// filenames
char *infile = "-";
char *outfile = "-";

// files
FILE *in = NULL;
FILE *out = NULL;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- show banner
  if (args.verbose_given)
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
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  unsigned int i;

  GetMyOptions(argc,argv);

  if (args.inputs_num > 0) {
    for (i = 0; i < args.inputs_num; i++) {
      fprintf(out, "\n# %s: File: %s\n\n", PROGNAME, args.inputs[i]);
      if (!(in = fopen(args.inputs[i], "r"))) {
	fprintf(stderr, "%s: open failed for input-file '%s': %s", PROGNAME, args.inputs[i], strerror(errno));
	exit(1);
      }
      if (args.verbose_given) {
	fprintf(stderr,"%s: processing file '%s'... ", PROGNAME, args.inputs[i]);
      }
      dwdspp_tokenize_stream(in, out);
      fclose(in);
      if (args.verbose_given) {
	fprintf(stderr," done.\n");
      }
    }
  } else {
    /* parse stdin */
    fprintf(out, "\n# %s: File: %s\n\n", PROGNAME, "<stdin>");
    if (args.verbose_given) {
      fprintf(stderr,"%s: processing file <stdin>... ", PROGNAME);
    }
    dwdspp_tokenize_stream(stdin, out);
    if (args.verbose_given) {
      fprintf(stderr," done.\n");
    }
  }

  return 0;
}

