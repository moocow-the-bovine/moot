/*--------------------------------------------------------------------------
 * File: dwdst_main.cc
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

#include "dwdst_cmdparser.h"
#include "cmdutil.h"
#include "dwdst.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdst";

// options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

// -- filenames
char *symfile = "dwdst.sym";
char *fstfile = "dwdst.fst";

// -- files
cmdutil_file_info out;

// -- global classes/structs
dwds_tagger dwdst;

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
	    "\n%s version %s by Bryan Jurish <moocow@ling.uni-potsdam.de>\n\n",
	    PROGNAME, VERSION);

  // -- output file
  out.name = args.output_arg;
  if (strcmp(out.name,"-") == 0) out.name = "<stdout>";
  if (!out.open("w")) {
    fprintf(stderr,"%s: open failed for output-file '%s': %s\n",
	    PROGNAME, out.name, strerror(errno));
    exit(1);
  }

  // -- set up file-churner
  churner.progname = PROGNAME;
  churner.inputs = args.inputs;
  churner.ninputs = args.inputs_num;
  churner.use_list = args.list_given;


  // -- tagger object setup : flags
  dwdst.want_avm = args.avm_given;
  dwdst.verbose  = (args.verbose_arg > 0);

  // -- tagger object setup : symbols
  if (args.verbose_arg > 0) fprintf(stderr, "%s: loading symbols-file '%s'...", PROGNAME, args.symbols_arg);
  if (!dwdst.load_symbols(args.symbols_arg)) {
    fprintf(stderr,"\n%s: load FAILED for symbols-file '%s'\n", PROGNAME, args.symbols_arg);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," loaded.\n");
  }

  // -- tagger object setup : morphology FST
  if (args.verbose_arg > 0) fprintf(stderr, "%s: loading morphology FST '%s'...", PROGNAME, args.morph_arg);
  if (!dwdst.load_morph(args.morph_arg)) {
    fprintf(stderr,"\n%s: load FAILED for morphology FST '%s'\n", PROGNAME, args.morph_arg);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," loaded.\n");
  }

  // link morph-FST to symbols file
  if (!dwdst.morph->fsm_use_symbol_spec(dwdst.syms)) {
    fprintf(stderr,"%s ERROR: could not use symbols from '%s' in FST from '%s'\n",
	    PROGNAME, args.symbols_arg, args.morph_arg);
    exit(1);
  }
}
  


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  int nfiles = 0;
  timeval istarted, astarted, astopped;
  double ielapsed, aelapsed;

  // -- get initialization start-time
  if (args.verbose_arg > 0) gettimeofday(&istarted, NULL);

  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  if (args.verbose_arg > 0) gettimeofday(&astarted, NULL);


  // -- the guts
  if (args.words_given) {
    fprintf(out.file, "# %s: Analyzing command-line tokens\n\n", PROGNAME);
    dwdst.tag_strings(args.inputs_num, args.inputs, out.file);
  } else {
    // -- big loop
    for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
      if (args.verbose_arg > 0) {
	nfiles++;
	if (args.verbose_arg > 1) {
	  fprintf(stderr,"%s: analyzing file '%s'... ", PROGNAME, churner.in.name);
	  fflush(stderr);
	}
      }
      fprintf(out.file, "\n# %s: File: %s\n\n", PROGNAME, churner.in.name);
      
      dwdst.tag_stream(churner.in.file, out.file);
      
      if (args.verbose_arg > 1) {
	fprintf(stderr," done.\n");
	fflush(stderr);
      }
    }
    out.close();
  }

  // -- summary
  if (args.verbose_arg > 0) {
      // -- timing
      gettimeofday(&astopped, NULL);

      aelapsed = astopped.tv_sec-astarted.tv_sec + (astopped.tv_usec-astarted.tv_usec)/1000000.0;
      ielapsed = astarted.tv_sec-istarted.tv_sec + (astarted.tv_usec-istarted.tv_usec)/1000000.0;

      // -- print summary
      fprintf(stderr, "\n-----------------------------------------------------\n");
      fprintf(stderr, "%s Summary:\n", PROGNAME);
      fprintf(stderr, "  + Files processed  : %d\n", nfiles);
      fprintf(stderr, "  + Tokens processed : %d\n", dwdst.ntokens);
      fprintf(stderr, "  + Unknown tokens   : %d\n", dwdst.nunknown);
      fprintf(stderr, "  + Recognition Rate : ");
      if (dwdst.ntokens > 0) {
	// -- avoid div-by-zero errors
	fprintf(stderr, "%.2f\n", (double)(dwdst.ntokens-dwdst.nunknown)/(double)dwdst.ntokens);
      } else {
	fprintf(stderr, "-NaN-\n");
      }
      fprintf(stderr, "  + Init Time        : %.2f sec\n", ielapsed);
      fprintf(stderr, "  + Analysis Time    : %.2f sec\n", aelapsed);
      fprintf(stderr, "  + Throughput       : %.2f toks/sec\n", (float)dwdst.ntokens/aelapsed);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}

