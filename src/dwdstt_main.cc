/*--------------------------------------------------------------------------
 * File: dwdstt_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger-trainer for DWDS project : main()
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "dwdstt_cmdparser.h"
#include "cmdutil.h"
#include "dwdstt.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdstt";

// options & file-churning
gengetopt_args_info args;
cmdutil_file_churner churner;

// -- filenames
char *symfile = "dwdst.sym";
char *fstfile = "dwdst.fst";

// -- files
cmdutil_file_info out;

// -- global classes/structs
dwds_tagger_trainer dwdstt;

// -- for verbose timing info
/*
  timeval istarted, astarted, astopped;
  double ielapsed, aelapsed;
*/

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

  // -- get initialization start-time
  //if (args.verbose_arg > 0) gettimeofday(&istarted, NULL);

  // -- trainer object setup : flags
  //dwdst.want_avm = args.avm_given;
  dwdstt.want_avm = false;
  dwdstt.want_features = !args.tags_only_given;
  dwdstt.verbose  = (args.verbose_arg > 0);

  // -- trainer object setup : symbols
  if (args.verbose_arg > 0) fprintf(stderr, "%s: loading symbols-file '%s'...", PROGNAME, args.symbols_arg);
  if (!dwdstt.load_symbols(args.symbols_arg)) {
    fprintf(stderr,"\n%s: load FAILED for symbols-file '%s'\n", PROGNAME, args.symbols_arg);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," loaded.\n");
  }


  // -- trainer object setup : pos-tag-list
  if (args.verbose_arg > 0)
    fprintf(stderr, "%s: generating PoS-tag list from file '%s'...",
	    PROGNAME, args.pos_list_given ? args.pos_list_arg : "(none)");
  if (dwdstt.get_pos_tags(args.pos_list_given ? args.pos_list_arg : NULL).empty()) {
    fprintf(stderr,"\n%s: generation FAILED for PoS-tag-list from file '%s'\n",
	    PROGNAME, args.pos_list_given ? args.pos_list_arg : "(none)");
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," generated.\n");
  }

  // -- trainer object setup : unknown-analysis FSA
  if (args.verbose_arg > 0)
    fprintf(stderr, "%s: generating analysis FSA for unknown tokens...", PROGNAME);
  if (!dwdstt.generate_unknown_fsa()) {
    fprintf(stderr,"\n%s: generation FAILED for unknown-token analysis FSA!\n", PROGNAME);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," generated.\n");
  }

  // -- save unknown FSA
  if (args.unknown_output_given) {
    if (args.verbose_arg > 0) {
      fprintf(stderr, "%s: saving analysis FSA for unknown tokens to file '%s'...",
	      PROGNAME, args.unknown_output_arg);
    }
    FSM *ufsa_t = dwdstt.ufsa->fsm_convert(FSM::FSMCTTable);
    if (!ufsa_t->fsm_save_to_binary_file(args.unknown_output_arg)) {
      fprintf(stderr, "\n%s Error: could not save analysis FSA for unknown tokens to file '%s'.\n",
	      PROGNAME, args.unknown_output_arg);
      exit(1);
    } else if (args.verbose_arg > 0) {
      fprintf(stderr, " saved.\n");
    }
    ufsa_t->fsm_free();
    // ok, we're done here...
    exit(0);
  }

  // -- tagger-trainer object setup : morphology FST
  if (args.verbose_arg > 0) fprintf(stderr, "%s: loading morphology FST '%s'...", PROGNAME, args.morph_arg);
  if (!dwdstt.load_morph(args.morph_arg)) {
    fprintf(stderr,"\n%s: load FAILED for morphology FST '%s'\n", PROGNAME, args.morph_arg);
  } else if (args.verbose_arg > 0) {
    fprintf(stderr," loaded.\n");
  }

  // -- tagger-trainer object setup : kmax
  dwdstt.kmax = args.kgram_max_arg;

  // -- tagger-trainer object setup : link morph-FST to symbols file
  if (!dwdstt.morph->fsm_use_symbol_spec(dwdstt.syms)) {
    fprintf(stderr,"%s ERROR: could not use symbols from '%s' in FST from '%s'\n",
	    PROGNAME, args.symbols_arg, args.morph_arg);
    exit(1);
  }

  // -- report
  if (args.verbose_arg > 0) {
    fprintf(stderr, "%s: Initialization complete\n\n", PROGNAME);
  }
}
  


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  int nfiles = 0;

  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  //if (args.verbose_arg > 0) gettimeofday(&astarted, NULL);

  // -- the guts
  if (args.words_given) {
    if (args.verbose_arg > 0) {
      fprintf(stderr, "%s: Analyzing command-line tokens\n\n", PROGNAME);
    }
    dwdstt.train_from_strings(args.inputs_num, args.inputs, out.file);
  } else {
    if (args.verbose_arg > 0) {
      fprintf(stderr, "%s: Analyzing input files... %s",
	      PROGNAME, args.verbose_arg > 1 ? "\n" : "");
    }
    // -- big loop
    for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
      if (args.verbose_arg > 0) {
	nfiles++;
	if (args.verbose_arg > 1) {
	  fprintf(stderr,"%s: analyzing file '%s'... ", PROGNAME, churner.in.name);
	  fflush(stderr);
	}
      }
#    ifdef DWDSTT_DEBUG
      fprintf(out.file, "\n# %s (DEBUG): File: %s\n\n", PROGNAME, churner.in.name);
#    endif
      
      dwdstt.train_from_stream(churner.in.file, out.file);

      if (args.verbose_arg > 1) {
	fprintf(stderr," done.\n");
	fflush(stderr);
      }
    }
    if (args.verbose_arg == 1) {
      fprintf(stderr, " done.\n");
      fflush(stderr);
    }
  }

#ifdef DWDSTT_DEBUG
  fprintf(out.file, "\n");
#endif

  // -- paramater file
  if (args.verbose_arg > 0 && out.file != stdout) {
    fprintf(stderr, "%s: Generating parameter file '%s'... ",
	    PROGNAME, out.name);
  }
  dwdstt.write_param_file(out.file);
  if (args.verbose_arg > 0 && out.file != stdout) {
    out.close();
    fprintf(stderr, "done.\n");
  } else {
    out.close();
  }

  // -- summary
  if (args.verbose_arg > 0) {
      // -- print summary
      fprintf(stderr, "\n-----------------------------------------------------\n");
      fprintf(stderr, "%s Summary:\n", PROGNAME);
      fprintf(stderr, "  + Files processed  : %d\n", nfiles);
      fprintf(stderr, "  + Tokens processed : %d\n", dwdstt.ntokens);
      fprintf(stderr, "  + Unknown tokens   : %d\n", dwdstt.nunknown);
      fprintf(stderr, "  + Recognition Rate : ");
      if (dwdstt.ntokens > 0) {
	// -- avoid div-by-zero errors
	fprintf(stderr, "%.2f %%\n", 100.0*(double)(dwdstt.ntokens-dwdstt.nunknown)/(double)dwdstt.ntokens);
      } else {
	fprintf(stderr, "-NaN-\n");
      }
      //fprintf(stderr, "  + Initialize Time  : %.2f sec\n", ielapsed);
      //fprintf(stderr, "  + Analysis Time    : %.2f sec\n", aelapsed);
      //fprintf(stderr, "  + Throughput       : %.2f tok/sec\n", (float)dwdstt.ntokens/aelapsed);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}

