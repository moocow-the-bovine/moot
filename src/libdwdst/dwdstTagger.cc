/*--------------------------------------------------------------------------
 * File: dwdstTagger.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : the guts
 *--------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
# include <dwdstConfig.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include <string>
#include <set>
#include <list>

#include <FSMTypes.h>

#include "dwdstTaggerLexer.h"
#include "dwdstTagger.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwdstTagger::dwdstTagger(FSMSymSpec *mysmys=NULL, FSM *mymorph=NULL)
 */
dwdstTagger::dwdstTagger(FSMSymSpec *mysyms, FSM *mymorph)
{
  // -- public data
  morph = mymorph;
  syms = mysyms;
  eos = "--EOS--";
  wordStart = "wordStart";
  wordEnd = "wordEnd";
  ufsa = NULL;
  dfsa = NULL;
  dis  = NULL;

  // -- public flags
  want_avm = false;
  want_numeric = false;
  want_features = true;
  want_tnt_format = false;
  want_classes_only = false;
  verbose = 0;

  // -- private data
  i_made_morph = false;
  i_made_syms = false;
  i_made_ufsa = false;
  i_made_dfsa = false;

  // -- temporary variables
  tmp = new FSM();
  result = NULL;

  // -- files
  infile = NULL;
  outfile = NULL;
  srcname = NULL;
}

/*
 * FSMSymSpec *dwdstTagger::load_morph_symbols(const char *filename)
 */
FSMSymSpec *dwdstTagger::load_morph_symbols(char *filename)
{
  list<string> msglist;

  // -- cleanup old symbols first
  if (syms && i_made_syms) delete syms;

  syms = new FSMSymSpec(filename, &msglist, DWDST_SYM_ATT_COMPAT);
  if (!msglist.empty()) {
    fprintf(stderr,
	    "\ndwdstTagger::load_symbols() Error: could not load symbols from file '%s'\n",
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
 * FSM *dwdstTagger::load_fsm_file(const char *filename, FSM **fsm, bool *i_made_fsm=NULL);
 */
FSM *dwdstTagger::load_fsm_file(char *filename, FSM **fsm, bool *i_made_fsm=NULL)
{
  // -- cleanup old FSM first
  if (*fsm && i_made_fsm && *i_made_fsm) { delete *fsm; }

  *fsm = new FSM(filename);
  if (!**fsm) {
    fprintf(stderr,"\ndwdstTagger::load_fsm_file() Error: load failed for FST file '%s'.\n", filename);
    *fsm = NULL; // -- invalidate the object
  }
  if (i_made_fsm) { *i_made_fsm = true; }
  return *fsm;
}


/*
 * dwdstTagger::~dwdstTagger()
 */
dwdstTagger::~dwdstTagger() {
  if (tmp) delete tmp;
  if (syms && i_made_syms) delete syms;
  if (morph && i_made_morph) delete morph;
  if (ufsa && i_made_ufsa) delete ufsa;
  if (dfsa && i_made_dfsa) delete dfsa;

  syms = NULL;
  morph = NULL;
  ufsa = NULL;
  dfsa = NULL;
  result = NULL;
  tmp = NULL;
}

/*--------------------------------------------------------------------------
 * Low-Level Tagging Methods
 *--------------------------------------------------------------------------*/

/*
 * inline void dwdstTagger::tag_eos()
 *  + NON-DISAMBIGUATING MODE:
 *    - just writes eos string to this->outfile
 *  + DISAMBIGUATING MODE:
 *    - performs disambiguation via this->dis
 *    - writes sentence output to this->outfile
 *      based on this->sentence_words
 */
inline void dwdstTagger::tag_eos(void)
{
  if (dis) {
    // -- disambiguating mode
    if (dis->sentbuf.empty()) return; // -- ignore empty sentences
    if (!want_numeric)
      {
	// -- disambiguating, strings
	vector<const FSMSymbolString *> &dstags = dis->disambiguate_sentence_string();
	if (verbose>1) {
	  if (dis->flags&dwdstDisambiguator::dSentenceLookupFailed) {
	    carp("dwdstTagger: Warning: sentence-disambiguation failed");
	  } else if (dis->flags&dwdstDisambiguator::dSentenceLookupAmbig) {
	    carp("dwdstTagger: Warning: disambiguator returned ambiguous result");
	  }
	}
	dsti = dstags.begin();
	if (want_tnt_format)
	  {
	    // -- disambiguating, strings, 1 tok/line
	    for (sti = sentence_tokens.begin();
		 sti != sentence_tokens.end() && dsti != dstags.end();
		 sti++, dsti++)
	      {
		// -- output token-analyses
		fputs(sti->c_str(),outfile);
		fputs("\t ",outfile);
		fputs((*dsti)->c_str(),outfile);
		fputc('\n',outfile);
	      }
	    if (sti != sentence_tokens.end()) {
	      if (verbose>0) {
		carp("dwdstTagger: Warning: leftover tokens after disambiguator lookup");
	      }
	      for (; sti != sentence_tokens.end(); sti++)
		{
		  // -- output leftover tokens (this loop should never run)
		  fputs(sti->c_str(), outfile);
		  fputs("\t ", outfile);
		  fputs(dis->bottom.c_str(), outfile);
		  fputc('\n', outfile);
		}
	    }
	    // -- eos-marker is output below
	  }
	else 
	  { /*!want_tnt_format*/
	    // -- disambiguating, strings, madwds-format
	    for (sti = sentence_tokens.begin();
		 sti != sentence_tokens.end() && dsti != dstags.end();
		 sti++, dsti++)
	      {
		fputs(sti->c_str(), outfile);
		fputs(": 1 Analyse(n)\n\t", outfile);
		fputs((*dsti)->c_str(), outfile);
		fputs("\n\n", outfile);
	      }
	    if (sti != sentence_tokens.end()) {
	      carp("dwdstTagger: warning: leftover tokens after disambiguator lookup!\n");
	      for (; sti != sentence_tokens.end(); sti++)
		{
		  // -- output leftover tokens (this loop should never run)
		  fputs(sti->c_str(), outfile);
		  fputs(": 1 Analyse(n)\n\t", outfile);
		  fputs(dis->bottom.c_str(), outfile);
		  fputs("\n\n", outfile);
		}
	    }
	    // -- eos-marker is output below
	  }
      }
    else
      { /*!want_numeric*/
	// -- disambiguating, numeric
	const FSM::FSMSymbolVector &dntags = dis->disambiguate_sentence_vector();
	if (verbose > 1) {
	  if (dis->flags&dwdstDisambiguator::dSentenceLookupFailed) {
	    carp("dwdstTagger: Warning: sentence-disambiguation failed");
	  } else if (dis->flags&dwdstDisambiguator::dSentenceLookupAmbig) {
	    carp("dwdstTagger: Warning: disambiguator returned ambiguous result");
	  }
	}
	dnti = dntags.begin();
	if (want_tnt_format)
	  {
	    // -- disambiguating, numeric, 1 tok/line
	    for (sti = sentence_tokens.begin();
		 sti != sentence_tokens.end() && dnti != dntags.end();
		 sti++, dnti++)
	      {
		// -- output token-analyses
		fprintf(outfile, "%s\t %d\n", sti->c_str(), *dnti);
	      }
	    if (sti != sentence_tokens.end()) {
	      carp("dwdstTagger: warning: leftover tokens after disambiguator lookup!\n");
	      for (; sti != sentence_tokens.end(); sti++)
		{
		  // -- output leftover tokens (this loop should never run)
		  fprintf(outfile, "%s\t %d\n",
			  sti->c_str(),
			  dis->syms->symbolname_to_symbol(dis->bottom));
		}
	    }
	    // -- eos-marker is output below
	  }
	else
	  { /*!want_tnt_format*/
	    // -- disambiguating, numeric, madwds-format
	    for (sti = sentence_tokens.begin();
		 sti != sentence_tokens.end() && dnti != dntags.end();
		 sti++, dnti++)
	      {
		fprintf(outfile, "%s: 1 Analyse(n)\n\t%d\n\n", sti->c_str(), *dnti);
	      }
	    if (sti != sentence_tokens.end()) {
	      carp("dwdstTagger: warning: leftover tokens after disambiguator lookup!\n");
	      for (; sti != sentence_tokens.end(); sti++)
		{
		  // -- output leftover tokens (this loop should never run)
		  fprintf(outfile, "%s: 0 Analyse(n)\n\n", sti->c_str());
		}
	    }
	    // -- eos-marker is output below
	  }
      }
    sentence_tokens.clear();
    dis->new_sentence();
  } /* if (dis) */

  // -- ambiguous mode & disambiguating-mode
  if (want_tnt_format) {
    fputc('\n', outfile);
  } else {
    fputs(eos.c_str(), outfile);
    fputs("\n\n", outfile);
  }
}


/*
 * inline void dwdstTagger::tag_token(void)
 *  + tags the input-token in this->token
 *  + NON-DISAMBIGUATING MODE:
 *    - ouputs to this->outfile
 *    - formats according to this->want_* data members
 *  + DISAMBIGUATING MODE:
 *    - pushes word onto the literal sentence-buffer
 *    - feeds analyses to this->dis
 */
inline void dwdstTagger::tag_token(void)
{
  tmp->fsm_clear();

  if (dis) {
    // -- disambiguating
    sentence_tokens.push_back((char *)token);
    result = morph->fsm_lookup(sentence_tokens.back(),tmp,true);

    if (want_features) {
      /*-- disambiguating, all features */
      nresults.clear();
      tmp->fsm_symbol_vectors(nresults, false);
      if (verbose>0 && nresults.empty()) nunknown++;
      if (!dis->append_token_analyses(nresults) && verbose>1) {
	carp("dwdstTagger: Warning: class-lookup failed for '%s'", token);
      }
    } else {
      /*-- disambiguating, tags only */
      ntagresults.clear();
      get_fsm_tags(tmp, &ntagresults);
      if (verbose>0 && ntagresults.empty()) nunknown++;
      if (!dis->append_token_analyses(ntagresults) && verbose>1) {
	carp("dwdstTagger: Warning: class-lookup failed for '%s'", token);
      }
    }
  }
  else {
    // -- ambiguous
    s = (char *)token;
    result = morph->fsm_lookup(s,tmp,true);

    fputs(token, outfile);
    if (want_numeric) {
      /*-- ambiguous, numeric */
      if (want_features) {
	/*-- ambiguous, numeric, all features */
	nresults.clear();
	tmp->fsm_symbol_vectors(nresults, false);
	if (verbose && nresults.empty()) nunknown++;
	if (want_tnt_format) {
	  /*-- ambiguous, numeric, all features, one tok/line */
	  for (nri = nresults.begin(); nri != nresults.end(); nri++) {
	    fputs("\t", outfile);
	    for (svi = nri->istr.begin(); svi != nri->istr.end(); svi++) {
	      fprintf(outfile, "%d ", *svi);
	    }
	    if (nri->weight) fprintf(outfile, "<%f>", nri->weight);
	  }
	  fputc('\n',outfile);
	} else { /*-- want_tnt_format */
	  /*-- ambiguous, numeric, all features, madwds-style */
	  fprintf(outfile, ": %d Analyse(n)\n", nresults.size());
	  for (nri = nresults.begin(); nri != nresults.end(); nri++) {
	    fputs("\t", outfile);
	    for (svi = nri->istr.begin(); svi != nri->istr.end(); svi++) {
	      fprintf(outfile, "%d ", *svi);
	    }
	    if (nri->weight) fprintf(outfile, "\t<%f>", nri->weight);
	    fputc('\n',outfile);
	  }
	  fputc('\n',outfile);
	}
      }
      else { /* --want_features */
	/*-- ambiguous, numeric, tags only */
	ntagresults.clear();
	get_fsm_tags(tmp, &ntagresults);
	if (verbose && ntagresults.empty()) nunknown++;
	
	if (want_tnt_format) {
	  /*-- ambiguous, numeric, tags-only, one tok/line */
	  for (ntri = ntagresults.begin(); ntri != ntagresults.end(); ntri++) {
	    fprintf(outfile, "\t %d", *ntri);
	  }
	  fputc('\n', outfile);
	} else { /*-- want_tnt_format */
	  /*-- ambiguous, numeric, tags-only, madwds native format */
	  fprintf(outfile, ": %d Analyse(n)\n", ntagresults.size());
	  for (ntri = ntagresults.begin(); ntri != ntagresults.end(); ntri++) {
	    fprintf(outfile, "\t%d\n", *ntri);
	  }
	  fputc('\n',outfile);
	}
      }
    }
    else { /* --want_numeric */
      /*-- ambiguous, strings */
      if (want_features) {
	/*-- ambiguous, strings, all features */      
	results.clear();
	
	tmp->fsm_strings(*syms, results, false, want_avm);
	if (verbose && results.empty()) nunknown++;
	if (want_tnt_format) {
	  /*-- ambiguous, strings, all features, one tok/line */
	  for (ri = results.begin(); ri != results.end(); ri++) {
	    fputs("\t", outfile);
	    fputs(ri->istr.c_str(), outfile);
	    if (ri->weight) fprintf(outfile, "<%f>", ri->weight);
	  }
	  fputc('\n',outfile);
	} else { /*-- want_tnt_format */
	  /*-- ambiguous, strings, all features, madwds-style */
	  fprintf(outfile, ": %d Analyse(n)\n", results.size());
	  for (ri = results.begin(); ri != results.end(); ri++) {
	    fputs("\t", outfile);
	    fputs(ri->istr.c_str(), outfile);
	    fprintf(outfile, (ri->weight ? "\t<%f>\n" : "\n"), ri->weight);
	  }
	  fputc('\n',outfile);
	}
      }
      else { /*-- want_features */
	/*-- ambiguous, strings, tags only */
	tagresults.clear();
	get_fsm_tag_strings(tmp, &tagresults);
	if (verbose && tagresults.empty()) nunknown++;
	
	if (want_tnt_format) {
	  /*-- ambiguous, strings, tags-only, one tok/line */
	  for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	    fputc('\t', outfile);
	    fputs(want_avm && *(tri->c_str()) == '_'
		  ? tri->c_str()+1
		  : tri->c_str(),
		  outfile);
	  }
	  fputc('\n', outfile);
	} else { /*-- want_tnt_format */
	  /*-- ambiguous, strings, tags-only, madwds native format */
	  fprintf(outfile, ": %d Analyse(n)\n", tagresults.size());
	  for (tri = tagresults.begin(); tri != tagresults.end(); tri++) {
	    /*
	      fputs("\t[", outfile);
	      fputs(want_avm && *(tri->c_str()) == '_'
	      ? tri->c_str()+1
	      : tri->c_str(),
	      outfile);
	      fputs("]\n", outfile);
	    */
	    fputc('\t',outfile);
	    fputs(tri->c_str(), outfile);
	    fputc('\n', outfile);
	  }
	  fputc('\n',outfile);
	}
      }
    }
  }
}


/*--------------------------------------------------------------------------
 * High-Level Tagging Methods
 *--------------------------------------------------------------------------*/

/*
 * bool dwdstTagger::tag_stream(FILE *in, FILE *out, char *srcname)
 */
bool dwdstTagger::tag_stream(FILE *in, FILE *out, char *infilename)
{
  int tok;

  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwdstTagger::tag_stream(): cannot run uninitialized tagger!\n");
    return false;
  }

  // -- files
  infile = in;
  outfile = out;
  srcname = infilename;

  // -- lexer prep
  lexer.step_streams(in,out);
  lexer.theLine = 1;
  lexer.theColumn = 0;

  while ((tok = lexer.yylex()) != dwdstTaggerLexer::DTEOF) {
    switch (tok) {
    case dwdstTaggerLexer::EOS:
      /* do something spiffy */
      tag_eos();
      break;

    case dwdstTaggerLexer::TOKEN:
    default:
      if (verbose) ntokens++;

      tmp->fsm_clear();
      token = (char *)lexer.yytext;
      tag_token();
    }
  }

  // -- cleanup
  infile = NULL;
  outfile = NULL;
  srcname = NULL;

  return true;
}

/*
 * bool dwdstTagger::tag_strings(int argc, char **argv, FILE *out, char *infilename)
 */
bool dwdstTagger::tag_strings(int argc, char **argv, FILE *out, char *infilename)
{
  // -- sanity check
  if (!can_tag()) {
    fprintf(stderr, "dwdstTagger::tag_strings(): cannot run uninitialized tagger!\n");
    return false;
  }

  // -- files
  infile = NULL;
  outfile = out;
  srcname = infilename;
  
  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    // -- verbosity
    if (verbose) ntokens++;

    token = *argv;
    tag_token();
  }
  tag_eos();

  // -- cleanup
  infile = NULL;
  outfile = NULL;

  return true;
}


/*--------------------------------------------------------------------------
 * Low-level methods
 *--------------------------------------------------------------------------*/

/*
 * set<FSMSymbol> *dwdstTagger::get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL);
 */
set<FSMSymbol> *dwdstTagger::get_fsm_tags(FSM *fsa, set<FSMSymbol> *tags=NULL)
{
  FSMState q0;
  FSMTransitions *arcs;
  FSMTransition  *arc;
  FSMRepresentation *rep = fsa->fsm_representation();

  // -- ensure 'tags' is defined
  if (!tags) tags = new set<FSMSymbol>();

  // -- get start-state
  q0 = rep->start_state();
  if (q0 == FSMNOSTATE) return tags;

  // -- get input-labels for all transitions from q0
  //    --> WARNING: if you get an '<epsilon>' in here, you
  //                 DO NOT have all initial symbols!
  arcs = rep->transitions(q0,false);
  while ((arc = arcs->next_transition()) != NULL) {
    tags->insert(arc->ilabel);
  }
  rep->delete_transition_iterator(arcs,false);

  return tags;
}


/*
 * set<FSMSymbolString> *dwdstTagger::get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL);
 */
set<FSMSymbolString> *
dwdstTagger::get_fsm_tag_strings(FSM *fsa, set<FSMSymbolString> *tag_strings=NULL)
{
  get_fsm_tags(fsa,&fsm_tags_tmp);

  // -- ensure 'tag_strings' is defined
  if (!tag_strings) tag_strings = new set<FSMSymbolString>();

  for (set<FSMSymbol>::const_iterator t = fsm_tags_tmp.begin(); t != fsm_tags_tmp.end(); t++) {
      FSMSymbolString tagstr = *(syms->symbol_to_symbolname(*t));
      if (want_avm && tagstr[0] == '_') { tagstr.replace(0,1,"["); }
      else { tagstr.insert(0,"["); }
      tagstr.append("]");
      tag_strings->insert(tagstr);
  }

  fsm_tags_tmp.clear();
  return tag_strings;
}

/*--------------------------------------------------------------------------
 * Debugging Methods
 *--------------------------------------------------------------------------*/

/** Convert a symbol-vector to a numeric string */
string dwdstTagger::symbol_vector_to_string(const FSM::FSMSymbolVector v)
{
  string vs;
  char buf[256];
  for (FSM::FSMSymbolVector::const_iterator vi = v.begin(); vi != v.end(); vi++)
    {
      sprintf(buf,"%d",*vi);
      vs.append(buf);
    }
  return vs;
}

/** Stringify a token-analysis-set (weighted-vector version) */
string dwdstTagger::analyses_to_string(const set<FSM::FSMWeightedSymbolVector> &analyses)
{
  string s = "{";
  for (set<FSM::FSMWeightedSymbolVector>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      s += symbol_vector_to_string(asi->istr);
      if (!asi->ostr.empty()) {
	s += " : ";
	s += symbol_vector_to_string(asi->istr);
      }
      if (asi->weight != 0.0) {
	char buf[256];
	sprintf(buf, " <%g>", asi->weight);
	s += buf;
      }
    }
  s += "}";
  return s;
}


/** Stringify a token-analysis-set (weighted-string-version) */
string
dwdstTagger::analyses_to_string(const set<FSM::FSMStringWeight> &analyses)
{
  string s = "{";
  for (set<FSM::FSMStringWeight>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      s += asi->istr;
      if (!asi->ostr.empty() && asi->ostr != "") {
	s += " : ";
	s += asi->ostr;
      }
      if (asi->weight != 0.0) {
	char buf[256];
	sprintf(buf, " <%g>", asi->weight);
	s += buf;
      }
    }
  s += "}";
  return s;
}

/** Stringify a token-analysis-set (numeric-tags version) */
string
dwdstTagger::analyses_to_string(const set<FSMSymbol> &analyses)
{
  string s = "{";
  char buf[256];
  for (set<FSMSymbol>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      sprintf(buf,"%d", *asi);
      s += buf;
    }
  s += "}";
  return s;
}

/** Stringify a token-analysis-set (string-tags version) */
string
dwdstTagger::analyses_to_string(const set<FSMSymbolString> &analyses)
{
  string s = "{";
  for (set<FSMSymbolString>::const_iterator asi = analyses.begin();
       asi != analyses.end();
       asi++)
    {
      if (asi != analyses.begin()) s += ", ";
      s += *asi;
    }
  s += "}";
  return s;
}


/*--------------------------------------------------------------------------
 * Error reporting
 *--------------------------------------------------------------------------*/
void dwdstTagger::carp(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, " %s%s at line %d, column %d, near '%s'\n",
	  (srcname ? "in file " : ""),
	  (srcname ? srcname : ""),
	  lexer.theLine, lexer.theColumn, lexer.yytext);
}
