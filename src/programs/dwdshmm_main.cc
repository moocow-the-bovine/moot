/*--------------------------------------------------------------------------
 * File: dwdshmm_main.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + HMM PoS tagger/disambiguator for DWDS project : main()
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <string>

#include <dwdstHMM.h>
#include <dwdstLexfreqs.h>
#include <dwdstNgrams.h>

#include "cmdutil.h"
#include "hmmutil.h"
#include "dwdshmm_cmdparser.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *PROGNAME = "dwdshmm";

// options & file-churning
gengetopt_args_info  args;
cmdutil_file_churner churner;

// -- files
cmdutil_file_info out;

// -- global classes/structs
dwdstHMM      hmm;
dwdstNgrams   ngrams;
dwdstLexfreqs lexfreqs;

// -- for verbose timing info
timeval istarted, astarted, astopped;
double  ielapsed, aelapsed;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void GetMyOptions(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  // -- load environmental defaults
  cmdline_parser_envdefaults(&args);

  // -- show banner
  if (args.verbose_arg > 1)
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
  if (args.verbose_arg > 1) gettimeofday(&istarted, NULL);

  // -- assign "unknown" ids
  hmm.unknown_token_name(args.unknown_token_arg);
  hmm.unknown_tag_name(args.unknown_tag_arg);

  // -- parse model spec
  char *lexfile=NULL;
  char *ngfile=NULL;
  if (!hmm_parse_model(args.model_arg, &lexfile, &ngfile)) {
    fprintf(stderr, "%s: could not parse model specification '%s'\n",
	    PROGNAME, args.model_arg);
    exit(1);
  }

  // -- load model: lexical frequencies
  if (lexfile) {
    if (args.verbose_arg > 1)
      fprintf(stderr, "%s: loading lexical frequency file '%s'...", PROGNAME, lexfile);
    if (!lexfreqs.load(lexfile)) {
      fprintf(stderr,"\n%s: load FAILED for lexical frequency file '%s'\n",
	      PROGNAME, lexfile);
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," loaded.\n");
    }
  }

  // -- load model: n-gram frequencies
  if (ngfile) {
    if (args.verbose_arg > 1)
      fprintf(stderr, "%s: loading n-gram frequency file '%s'...", PROGNAME, ngfile);
    if (!ngrams.load(ngfile)) {
      fprintf(stderr,"\n%s: load FAILED for n-gram frequency file '%s'\n",
	      PROGNAME, ngfile);
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," loaded.\n");
    }
  }

  // -- parse n-gram smoothing constants (nlambdas)
  if (args.nlambdas_arg) {
    double nlambdas[3] = {0,1,0};
    if (!hmm_parse_doubles(args.nlambdas_arg, nlambdas, 3)) {
      fprintf(stderr, "%s: could not parse N-Gram smoothing constants '%s'\n",
	      PROGNAME, args.nlambdas_arg);
      exit(1);
    }
    hmm.nglambda1 = nlambdas[0];
    hmm.nglambda2 = nlambdas[1];
#ifdef DWDST_USE_TRIGRAMS
    hmm.nglambda3 = nlambdas[2];
#endif
  } else {
    if (args.verbose_arg > 1)
      fprintf(stderr, "%s: estimating lambdas...", PROGNAME);
    if (!hmm.estimate_lambdas(ngrams)) {
      fprintf(stderr,"\n%s: lambda estimation FAILED.\n", PROGNAME);
      exit(1);
    } else if (args.verbose_arg > 1) {
      fprintf(stderr," done.\n");
    }
  }

  // -- parse lexical smoothing constants (wlambdas)
  if (args.wlambdas_arg) {
    double wlambdas[2] = {1,0};
    if (!hmm_parse_doubles(args.wlambdas_arg, wlambdas, 2)) {
      fprintf(stderr, "%s: could not parse lexical smoothing constants '%s'\n",
	      PROGNAME, args.wlambdas_arg);
      exit(1);
    }
    hmm.wlambda1 = wlambdas[0];
    hmm.wlambda2 = wlambdas[1];
  }

  // -- compile HMM
  if (args.verbose_arg > 1)
    fprintf(stderr, "%s: compiling HMM...", PROGNAME);
  if (!hmm.compile(lexfreqs, ngrams, args.eos_tag_arg, args.unknown_threshhold_arg)) {
    fprintf(stderr,"\n%s: HMM compilation FAILED\n", PROGNAME);
    exit(1);
  } else if (args.verbose_arg > 1) {
    fprintf(stderr," compiled.\n");
  }

  // -- report
  if (args.verbose_arg > 1) {
    fprintf(stderr, "%s: Initialization complete\n", PROGNAME);
    fprintf(stderr, "%s:   Freq. Threshhold : %g\n", PROGNAME, args.unknown_threshhold_arg);
    fprintf(stderr, "%s:   Unknown Token    : %s\n", PROGNAME, args.unknown_token_arg);
    fprintf(stderr, "%s:   Unknown Tag      : %s\n", PROGNAME, args.unknown_tag_arg);
    fprintf(stderr, "%s:   Border Tag       : %s\n", PROGNAME, args.eos_tag_arg);
    fprintf(stderr, "%s:   N-Gram lambdas   : lambda1=%g, lambda2=%g",
	    PROGNAME, hmm.nglambda1, hmm.nglambda2);
#ifdef DWDST_USE_TRIGRAMS
      fprintf(stderr, " lambda3=%g", hmm.nglambda3);
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "%s:   Lexical lambdas  : lambdaw1=%g, lambdaw2=%g\n",
	    PROGNAME, hmm.wlambda1, hmm.wlambda2);
    fprintf(stderr, "\n");
  }
}
  


/*--------------------------------------------------------------------------
 * main
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  size_t nfiles = 0;

  GetMyOptions(argc,argv);

  // -- get init-stop time = analysis-start time
  if (args.verbose_arg > 1) gettimeofday(&astarted, NULL);

  // -- the guts
  for (churner.first_input_file(); churner.in.file; churner.next_input_file()) {
    if (args.verbose_arg > 1) {
      nfiles++;
      if (args.verbose_arg > 2) {
	fprintf(stderr,"%s: analyzing file '%s'... ", PROGNAME, churner.in.name);
	fflush(stderr);
      }
    }
    if (args.verbose_arg > 1) {
      fprintf(out.file, "\n%%%% %s: File: %s\n\n", PROGNAME, churner.in.name);
    }

    hmm.tag_stream(churner.in.file, out.file, churner.in.name);
    
    if (args.verbose_arg > 2) {
      fprintf(stderr," done.\n");
      fflush(stderr);
    }
  }
  out.close();

  // -- summary
  if (args.verbose_arg > 1) {
      // -- timing
      gettimeofday(&astopped, NULL);

      aelapsed = astopped.tv_sec-astarted.tv_sec + (double)(astopped.tv_usec-astarted.tv_usec)/1000000.0;
      ielapsed = astarted.tv_sec-istarted.tv_sec + (double)(astarted.tv_usec-istarted.tv_usec)/1000000.0;

      // -- print summary
      fprintf(stderr, "\n-----------------------------------------------------\n");
      fprintf(stderr, "%s Summary:\n", PROGNAME);
      fprintf(stderr, "  + General\n");
      fprintf(stderr, "    - Files Processed     : %u files\n", nfiles);
      fprintf(stderr, "    - Tokens Processed    : %u tok\n", hmm.ntokens);
      fprintf(stderr, "    - Initialize Time     : %.3f sec\n", ielapsed);
      fprintf(stderr, "    - Analysis Time       : %.3f sec\n", aelapsed);
      fprintf(stderr, "    - Throughput Rate     : %.2f tok/sec\n", (double)hmm.ntokens/aelapsed);
      fprintf(stderr, "-----------------------------------------------------\n");
  }

  return 0;
}

