/*--------------------------------------------------------------------------
 * File: dwdstDisambiguator.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : disambiguation guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

#include <string>
#include <set>
#include <list>

#include <FSMTypes.h>

#include "dwdstDisambiguator.h"
#include "dwdstAlphabetCompiler.h"

using namespace std;

//#define DWDSTD_DEBUG 1
//#define DWDSTD_VERBOSE_ERRORS 1

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwdstDisambiguator::dwdstDisambiguator(FSMSymSpec *mysmys=NULL, FSM *mydfst=NULL)
 */
dwdstDisambiguator::dwdstDisambiguator(FSMSymSpec *mysyms, FSM *mydfst)
{
  // -- public data
  dfst = mydfst;
  syms = mysyms;
  /*
  bottom_class = "BOTTOM";
  bottom_tag   = "BOTTOM";
  */
  bottom = "BOTTOM";

  // -- flags
  verbose = 0;
  nsentences = 0;
  nunknown_classes = 0;
  nunknown_sentences = 0;
  nambig_sentences = 0;
  flags = 0;

  // -- protected data
  i_made_dfst = false;
  i_made_syms = false;

  // -- temporary variables
  tmp = new FSM();
  result = NULL;
}

/*
 * dwdstDisambiguator::~dwdstDisambiguator()
 */
dwdstDisambiguator::~dwdstDisambiguator() {
  if (tmp) delete tmp;
  if (syms && i_made_syms) delete syms;
  if (dfst && i_made_dfst) delete dfst;

  class2sym.clear();
  sentbuf.clear();

  syms = NULL;
  dfst = NULL;
  result = NULL;
  tmp = NULL;
}


/*--------------------------------------------------------------------------
 * Initialization
 *--------------------------------------------------------------------------*/

/*
 * FSMSymSpec *dwdstDisambiguator::load_symbols_file(const char *filename)
 */
FSMSymSpec *dwdstDisambiguator::load_symbols_file(char *filename)
{
  list<string> msglist;

  // -- cleanup old symbols first
  if (syms && i_made_syms) delete syms;

  syms = new FSMSymSpec(filename, &msglist, DWDST_SYM_ATT_COMPAT);
  if (!msglist.empty()) {
    fprintf(stderr,
	    "\ndwdstDisambiguator::load_symbols() Error: could not load symbols from file '%s'\n",
	    filename);
    for (list<string>::iterator e = msglist.begin(); e != msglist.end(); e++) {
      fprintf(stderr,"%s\n",e->c_str());
    }
    syms = NULL; // -- invalidate the object
  }

  i_made_syms = true;
  return syms;
}

/*
 * FSM *dwdstDisambiguator::load_fsm_file(char *filename);
 */
FSM *dwdstDisambiguator::load_fsm_file(char *filename)
{
  // -- cleanup old FSM first
  if (dfst && i_made_dfst) {
    delete dfst;
    dfst = NULL; 
  }

  dfst = new FSM(filename);
  if (!dfst || !*dfst) {
    fprintf(stderr,
	    "\ndwdstDisambiguator::load_fsm_file() Error: load failed for FST file '%s'.\n",
	    filename);
    dfst = NULL; // -- invalidate the object
  }
  i_made_dfst = true;
  return dfst;
}

dwdstSymbolVector2SymbolMap *
dwdstDisambiguator::load_alphabet_file(char *filename,
				       FSMSymSpec *input_symbols,
				       bool use_short_classnames=true)
{
  dwdstAlphabetCompiler acomp = dwdstAlphabetCompiler(input_symbols, syms, &class2sym);
  acomp.use_short_classnames = use_short_classnames;

  acomp.theLexer.theLine = 1;
  acomp.theLexer.theColumn = 1;

  FILE *afile = fopen(filename,"r");
  if (!afile) {
    fprintf(stderr,
	    "dwdstDisambiguator::load_alphabet_file(): open failed for file '%s': %s\n",
	    filename, strerror(errno));
    return NULL;
  }

  if (!acomp.parse_from_file(afile,filename)) {
    fclose(afile);
    return NULL;
  }

  fclose(afile);
  return &class2sym;
}


/*--------------------------------------------------------------------------
 * Tagging Methods
 *--------------------------------------------------------------------------*/

/*
 * Append the next token to the sentence buffer (weighted-vector version) [UNSAFE]
 */
bool
dwdstDisambiguator::append_token_analyses(const set<FSM::FSMWeightedSymbolVector> &analyses)
{
  flags &= ~dClassLookupFailed;
#if defined(DWDSTD_DEBUG)
  if (verbose>2) dump_sentence_vector("pre-append", sentbuf, stderr);
#endif

  if (!analyses.empty()) {
    class_key.clear();
    for (wsvsi = analyses.begin(); wsvsi != analyses.end(); wsvsi++) {
      class_key.insert(class_key.end(), wsvsi->istr.begin(), wsvsi->istr.end());
    }
    cv2si = class2sym.find(class_key);
    if (cv2si != class2sym.end()) {
      sentbuf.push_back(cv2si->second);
#    ifdef DWDSTD_DEBUG
      if (verbose>2) {
	dump_symbol_vector("succeeded-weighted", class_key, stderr);
	dump_sentence_vector("post-append", sentbuf, stderr);
      }
#    endif
      return true;
    }
  } 


  // -- don't know anything about this class: use universal-class
  flags |= dClassLookupFailed;
  sentbuf.push_back(syms->symbolname_to_symbol(bottom));

  if (verbose>0) {
    if (!analyses.empty()) nunknown_classes++;
#    if defined(DWDSTD_DEBUG)
    if (verbose>2) {
      dump_symbol_vector("failed-weighted", class_key, stderr);
      dump_sentence_vector("post-append", sentbuf, stderr);
    }
#    endif
  }
  return false;
}

/*
 * Append the next token to the sentence buffer (unweighted-vector version) [safer]
 */
bool
dwdstDisambiguator::append_token_analyses(const set<FSM::FSMSymbolVector> &analyses)
{
  flags &= ~dClassLookupFailed;
#ifdef DWDSTD_DEBUG
  if (verbose > 2) dump_sentence_vector("pre-append", sentbuf, stderr);
#endif

  if (!analyses.empty()) {
    class_key.clear();
    for (usvsi = analyses.begin(); usvsi != analyses.end(); usvsi++) {
      class_key.insert(class_key.end(), usvsi->begin(), usvsi->end());
    }
    cv2si = class2sym.find(class_key);
    if (cv2si != class2sym.end()) {
      sentbuf.push_back(cv2si->second);
#    ifdef DWDSTD_DEBUG
      if (verbose>2) {
	dump_symbol_vector("succeeded-unweighted", class_key, stderr);
	dump_sentence_vector("post-append", sentbuf, stderr);
      }
#    endif
      return true;
    }
  }

  // -- don't know anything about this class: use universal-class
  flags |= dClassLookupFailed;
  sentbuf.push_back(syms->symbolname_to_symbol(bottom));

  if (verbose>0) {
    if (!analyses.empty()) nunknown_classes++;
#  ifdef DWDSTD_DEBUG
    if (verbose>2) {
      dump_symbol_vector("failed-unweighted", class_key, stderr);
      dump_sentence_vector("post-append", sentbuf, stderr);
    }
#  endif
  }

  return false;
}


/*
 * Append the next token to the sentence buffer (unweighted-vector version) [even safer]
 */
bool
dwdstDisambiguator::append_token_analyses(const set<FSMSymbol> &analyses)
{
  flags &= ~dClassLookupFailed;
#ifdef DWDSTD_DEBUG
  if (verbose>2) dump_sentence_vector("pre-append", sentbuf, stderr);
#endif

  if (!analyses.empty()) {
    class_key.clear();
    for (ssi = analyses.begin(); ssi != analyses.end(); ssi++) {
      class_key.push_back(*ssi);
    }
    cv2si = class2sym.find(class_key);
    if (cv2si != class2sym.end()) {
      sentbuf.push_back(cv2si->second);
#    ifdef DWDSTD_DEBUG
      if (verbose>2) {
	dump_symbol_vector("succeeded-tags", class_key, stderr);
	dump_sentence_vector("post-append", sentbuf, stderr);
      }
#    endif
      return true;
    }
  }

  // -- don't know anything about this class: use universal-class
  flags |= dClassLookupFailed;
  sentbuf.push_back(syms->symbolname_to_symbol(bottom));

  if (verbose>0) {
    if (!analyses.empty()) nunknown_classes++;
#  ifdef DWDSTD_DEBUG
    if (verbose>2) {
      dump_symbol_vector("failed-tags", class_key, stderr);
      dump_sentence_vector("post-append", sentbuf, stderr);
    }
#  endif
  }
  return false;
}



/*
 * Disambiguate the current sentence buffer; output strings.
 */
vector<const FSMSymbolString *> &dwdstDisambiguator::disambiguate_sentence_string(void)
{
  flags &= ~(dSentenceLookupFailed|dSentenceLookupAmbig);
  tmp->fsm_clear();
  nresults.clear();
  string_result.clear();
  result = dfst->fsm_lookup_vector(sentbuf,tmp,true);
  tmp->fsm_symbol_vectors(nresults, false);

  if (nresults.empty()) {
    if (verbose > 0) {
      nunknown_sentences++;
#  ifdef DWDSTD_DEBUG
      if (verbose > 2) dump_sentence_vector("failed", sentbuf, stderr);
#  endif
    }
    
    // -- oops: lookup failed: initialize results to 'BOTTOM'^n
    flags |= dSentenceLookupFailed;
    string_result.insert(0, sentbuf.size(), &bottom);
  } else {
    // -- we have output : take the first result in the set
    nri = nresults.begin();
    for (svi = nri->istr.begin(); svi != nri->istr.end(); svi++) {
      string_result.push_back(syms->symbol_to_symbolname(*svi));
    }

    if (nresults.size() > 1) {
      flags |= dSentenceLookupAmbig;
      if (verbose > 0) nambig_sentences++;
    }
  }
  return string_result;
}


/*
 * Disambiguate the current sentence buffer; output vectors
 */
const FSM::FSMSymbolVector &dwdstDisambiguator::disambiguate_sentence_vector(void)
{
  flags &= ~(dSentenceLookupFailed|dSentenceLookupAmbig);
  tmp->fsm_clear();
  nresults.clear();
  result = dfst->fsm_lookup_vector(sentbuf,tmp,true);
  tmp->fsm_symbol_vectors(nresults, false);

  if (nresults.empty()) {

    if (verbose>0) {
      nunknown_sentences++;
#    ifdef DWDSTD_DEBUG
      if (verbose > 2) {
	dump_sentence_vector("failed", sentbuf, stderr);
      }
#    endif
    }

    // -- oops: lookup failed: initialize all results 'BOTTOM'^n
    flags |= dSentenceLookupFailed;
    vector_result.clear();
    vector_result.insert(0, (size_t)sentbuf.size(), syms->symbolname_to_symbol(bottom));
    return vector_result;
  }

  if (nresults.size() > 1) {
    flags |= dSentenceLookupAmbig;
    if (verbose > 0) nambig_sentences++;
  }
  
  // -- we have output : take the first result in the set
  nri = nresults.begin();
  return nri->istr;
}

/*--------------------------------------------------------------------------
 * Debugging Methods
 *--------------------------------------------------------------------------*/
void dwdstDisambiguator::dump_class_map(FILE *file=NULL)
{
  if (!file) file=stdout;
  for (cv2si = class2sym.begin(); cv2si != class2sym.end(); cv2si++) {
    for (svi = cv2si->first.begin(); svi != cv2si->first.end(); svi++) {
      fprintf(file, " %d", *svi);
    }
    fprintf(file, " = %d (%s) ",
	    cv2si->second,
	    syms->symbol_to_symbolname(cv2si->second)->c_str());
    fprintf(file,"\n");
  }
}

void dwdstDisambiguator::dump_symbol_vector(const char *label,
					    const FSM::FSMSymbolVector v,
					    FILE *file=NULL)
{
  if (!file) file=stdout;
  fprintf(file, "<dump(%s)>: vector=", label);
  for (FSM::FSMSymbolVector::const_iterator vi = v.begin(); vi != v.end(); vi++)
    {
      fprintf(file, "%d ", *vi);
    }
  fprintf(file, "\n");
}

void dwdstDisambiguator::dump_sentence_vector(const char *label,
					      const FSM::FSMSymbolVector s,
					      FILE *file=NULL)
{
  if (!file) file=stdout;
  fprintf(file, "<dump-sentence(%s)>: vector=", label);
  for (FSM::FSMSymbolVector::const_iterator si = s.begin(); si != s.end(); si++)
    {
      fprintf(file, "%d ", *si);
    }
  fprintf(file, "\n<dump-sentence(%s)>: string=", label);
  for (FSM::FSMSymbolVector::const_iterator si = s.begin(); si != s.end(); si++)
    {
      const FSMSymbolString *symname = syms->symbol_to_symbolname(*si);
      fprintf(file, "%s ", symname ? symname->c_str() : "NULL");
    }
  fprintf(file, "\n");
}
