#include "chashtest.h"

/*----------------------------------------------------------------------
 * Class-probability lookup table
 *----------------------------------------------------------------------*/
#define BENCH_CLASS dwdstCHMM
class BENCH_CLASS : public dwdstHMM {
public:
  /** Typedef for a class-ID */
  typedef set<FSM::FSMSymbolVector> LexClass;

  /** Typedef for a class probability lookup table (see also dwdstHMM::LexProbSubTable) */
  typedef map<TagID,ProbT> LexClassProbSubTable;

  /** Utility struct for hash_map<> */
  class LexClassHash {
  public:
    inline size_t operator()(const LexClass &x) const {
      size_t hv = 0;
      for (LexClass:const_iterator xi = x.begin(); xi != x.end(); xi++) {
	hv = 5*hv + *xi;
      }
      return hv;
    };
  };
  /** Utility struct for hash_map<> */
  class LexClassEqual {
  public:
    inline size_t operator()(const LexClass &x, const LexClass &y) const {
      return x==y;
    };
  };

  /** Typedef for a class-probability lookup table */
  typedef
    hash_map<ClassID,
	     LexClassProbSubTable,
	     LexClassHash,
	     LexClassEqual>
    LexClassProbTable;

public:
  //-- public data
  LexClassProbTable cprobs;

  //-- morphological tagger
  dwdstTagger tgr;

public:
  //-- public methods
  BENCH_CLASS(void) {};
  ~BENCH_CLASS(void) {};

  //-- compilation
  bool compile(const dwdstLexfreqs &lexfreqs,
	       const dwdstNgrams   &ngrams,
	       const dwdstTagString &start_tag_str=dwdstTagString(),
	       const dwdstLexfreqs::LexfreqCount unknownLexThreshhold=1);
}

bool BENCH_CLASS::compile(const dwdstLexfreqs &lexfreqs,
			  const dwdstNgrams   &ngrams,
			  const dwdstTagString &start_tag_str,
			  const dwdstLexfreqs::LexfreqCount unknownLexThreshhold)
{
  //-- superclass compilation
  if (!dwdstHMM::compile(lexfreqs,ngrams,start_tag_str,unknownLexThreshhold))
    return false;

  //-- save values
  int tgr_verbose = tgr.verbose;
  tgr.verbose     = 0;

  //-- iterate: lexical frequencies
  dwdstLexfreqs::LexfreqStringTable::const_iterator lfti;
  LexClass lclass;
  for (lfti = lexfreqs.lftable.begin(); lfti != lexfreqs.lftable.end(); lfti++)
    {
      const dwdstTokString &tokstr = tgr.tag_token(lfti->first.first);
      const dwdstTokString &tagstr = tgr.tag_token(lfti->first.second);
      const dwdstLexfreqs::LexfreqCount toktotal = lexfreqs.lookup(tokstr);
      const dwdstLexfreqs::LexfreqCount tagtotal = lexfreqs.taglookup(tagstr);

      //-- sanity check
      if (toktotal == 0 || tagtotal == 0) continue;

      //-- analyze token : get lexical class
      const dwdstTagger::MorphAnalysisSet &ma = tgr.tag_token(tokstr);
      lclass.clear();
      for (dwdstTagger::MorphAnalysisSet::const_iterator mai = ma.begin();
	   mai != ma.end();
	   mai++)
	{
	  lclass.insert(mai->istr);
	}

      //-- get TagID
      tagid = tagids.name2id(tagstr);

      //-- it's a kosher token (hack?)
      cprobs[tokid][tagid] = lfti->second / tagtotal;
    }

  //-- cleanup
  tgr.verbose = tgr_verbose;
  return true;
}

/*----------------------------------------------------------------------
 * MAIN
 *----------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  progname = *argv;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s LEXFREQS NGRAMS\n", progname);
    exit(1);
  }
  char *lexfile = *(++argv);
  char *ngfile  = *(++argv);


  //-- load symbols
  fprintf(stderr, "%s: loading morphological symbols '%s'...", progname, SYM_FILE);
  if (!tgr.load_morph_symbols(SYM_FILE)) {
    fprintf(stderr, "\nFAILED.\n");
    exit(1);
  } else {
    fprintf(stderr, " loaded.\n");
  }

  //-- load morphology
  fprintf(stderr, "%s: loading morphology FST '%s'...", progname, MORPH_FILE);
  if (!tgr.load_morph(MORPH_FILE)) {
    fprintf(stderr, "\nFAILED.\n");
    exit(1);
  } else {
    fprintf(stderr, " loaded.\n");
  }

  //-- load lexfreqs
  fprintf(stderr, "%s: loading lexical frequencies from '%s'... ", progname, lexfile);
  if (!lexfreqs.load(lexfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(2);
  }
  fprintf(stderr, "loaded.\n");

  //-- load ngrams
  fprintf(stderr, "%s: loading ngram frequencies from '%s'... ", progname, ngfile);
  if (!ngrams.load(ngfile)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "loaded.\n");

  //-- estimate lambdas
  fprintf(stderr, "%s: estimating lambdas...", progname);
  if (!hmm.estimate_lambdas(ngrams)) {
    fprintf(stderr, "FAILED.\n");
    exit(4);
  } else {
    fprintf(stderr, " done.\n");
  }

  //-- compile HMM
  fprintf(stderr, "%s: compiling HMM... ", progname);
  if (!hmm.compile(lexfreqs, ngrams, "@UNKNOWN", UNKNOWN_LEX_THRESH)) {
    fprintf(stderr, "FAILED.\n");
    exit(3);
  }
  fprintf(stderr, "compiled.\n");

  return 0;
}
