/*--------------------------------------------------------------------------
 * File: dwdst_trainer.cc
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + Trainer for KDWDS PoS-tagger: the guts
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>

#include "dwdst_trainer.h"
#include "postag_lexer.h"
//#include "dwdst_param_lexer.h"

using namespace std;

/*--------------------------------------------------------------------------
 * Constructor / Destructor
 *--------------------------------------------------------------------------*/

/*
 * dwds_tagger_trainer::~dwds_tagger_trainer()
 */
dwds_tagger_trainer::~dwds_tagger_trainer()
{
  // -- do nothing
  ;
}

/*--------------------------------------------------------------------------
 * Public Methods: mid-level: tag-list file-reading
 *--------------------------------------------------------------------------*/

/*
 * set<FSMSymbolString> dwds_tagger_trainer::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL);
 *  + implicitly clears 'tagset'
 *  + if 'filename' is NULL, 'tagset' is set to all categories according to 'syms' data member
 */
set<FSMSymbolString> dwds_tagger_trainer::read_taglist_file(set<FSMSymbolString> &tagset, const char *filename=NULL)
{
  // -- sanity check
  tagset.clear();

  if (!filename) {
    // -- default: all categories
    if (!syms) {
      fprintf(stderr, "dwds_tagger_trainer::read_taglist_file(): no symbols loaded!\n");
      return tagset;
    }
    const set<FSMSymbolString> *symbols = syms->symbols();
    for (set<FSMSymbolString>::iterator p = symbols->begin(); p != symbols->end(); p++) {
      if (!syms->is_category(*p)) continue;
      tagset.insert("_"+(*p));
    }
  } else {
    // -- load from file
    FILE *pos_file;
    postag_lexer lexer;
    FSMSymbolString s;
    int  tok;
    
    if (!(pos_file = fopen(filename,"r"))) {
      fprintf(stderr,"dwds_tagger_trainer::read_taglist_file(): could not open file '%s' for read.\n",
	      filename);
      return tagset;
    }
    // -- parse the pos-tag file
    lexer.step_streams(pos_file,stdout);
    while ((tok = lexer.yylex()) != PTEOF) {
      switch (tok) {
      case POSTAG:
	s = (char *)lexer.yytext;
	tagset.insert(s);
      case PTEOF:
	break;
      default:
	fprintf(stderr,"dwds_tagger_trainer::get_pos_tags(): Error in input file '%s'.\n",
		filename);
      }
    }
  }
  return tagset;
}


/*--------------------------------------------------------------------------
 * Public Methods: FSA Generation: Unknown-Analysis FSA
 *--------------------------------------------------------------------------*/

/*
 * FSM *dwds_tagger_trainer::generate_unknown_fsa();
 *   + generates analysis-FSA for tokens unknown to KDWDS morphology
 *   + uses 'opentags' : set of open-class PoS tags
 */
FSM *dwds_tagger_trainer::generate_unknown_fsa()
{
  FSMRepresentation *base;
  FSMState q0;
  set<FSMState> pstates;
  set<FSMState>::iterator ps;

  // -- ensure that ufsa exists
  if (ufsa) ufsa->fsm_clear();
  else {
    ufsa = new FSM();
    ufsa->set_fsm_type(false,false);
  }

  // -- add start state
  base = ufsa->fsm_representation();
  q0 = base->set_start_state(base->add_state(base->new_state()));

  // -- return an FSA ambiguous between all tags in 'tagset'
  for (set<FSMSymbolString>::iterator p = opentags.begin(); p != opentags.end(); p++) {
    pstates = fsm_add_pos_arc(ufsa, q0, *p, FSM_default_cost_structure.freecost());
    for (ps = pstates.begin(); ps != pstates.end(); ps++) {
      base->mark_state_as_final(*ps);
    }
  }
  return ufsa;
}

/*--------------------------------------------------------------------------
 * Public Methods: FSA Generation: Diambiguation-FSA
 *  --> NOT YET IMPLEMENTED!
 *--------------------------------------------------------------------------*/

/*
 * FSM *dwds_tagger_trainer::generate_disambig_fsa();
 */
FSM *dwds_tagger_trainer::generate_disambig_fsa()
{
  fprintf(stderr,"dwds_tagger_trainer::generate_disambig_fsa(): not yet implemented!");
  abort();
  return NULL;
}


/*--------------------------------------------------------------------------
 * Public Methods: mid-/low-level FSA Generation: arc-addition
 *--------------------------------------------------------------------------*/

/*
 * set<FSMState>
 * dwds_tagger_trainer::fsm_add_pos_arc(FSM *fsm, FSMState qfrom, FSMSymbolString &pos,
 *                                      FSMWeight cost = FSM_default_cost_structure.freecost())
 *   + returns set of "final" states for the arc
 *   + honors the 'want_features' data-member
 */
set<FSMState>
dwds_tagger_trainer::fsm_add_pos_arc(FSM *fsm, const FSMState qfrom, const FSMSymbolString &pos,
				     const FSMWeight cost = FSM_default_cost_structure.freecost())
{
  FSMRepresentation *base = fsm->fsm_representation();
  FSMState qi;
  set<FSMState> pstates;

  // -- return an FSA ambiguous between all tags in 'tagset' -- no features!
  qi = base->add_state(base->new_state());
  base->add_transition(qfrom, qi,
		       syms->symbolname_to_symbol(pos), 
		       syms->symbolname_to_symbol(pos),
		       cost);
  pstates.insert(qi);

  if (want_features) {
    FSMSymbolString catstr = pos;
    if (catstr[0] == '_') { catstr.erase(0,1); }
    const vector<FSMSymbolString> *feats = syms->features_of_category(catstr);
    FSMState qf; // -- feature-state

    for (vector<FSMSymbolString>::const_iterator f = feats->begin(); f != feats->end(); f++) {
      set<FSMSymbol> *values = syms->subtypes_of(*f);
      qf = base->add_state(base->new_state());
      for (set<FSMSymbol>::iterator v = values->begin(); v != values->end(); v++) {
	for (set<FSMState>::iterator ps = pstates.begin(); ps != pstates.end(); ps++) {
	  base->add_transition(*ps, qf, *v, *v, FSM_default_cost_structure.freecost());
	}
      }
      pstates.insert(qf);
    }
  }
  return pstates;
}


/*--------------------------------------------------------------------------
 * Public Methods: Parameter-generation: Training
 *--------------------------------------------------------------------------*/

/*
 * bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
 */
bool dwds_tagger_trainer::train_from_stream(FILE *in, FILE *out)
{
  int tok, eosi;

  // -- sanity check
  if (!can_tag() || !init_training_temps(in,out)) {
    fprintf(stderr, "dwds_tagger_trainer::train_from_stream(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- ye olde guttes
  lexer.step_streams(in,out);
  while ((tok = lexer.yylex()) != DTEOF) {
    // -- check for eos
    if (tok == EOS || tok == DTEOF) {
      eosi = kmax;
      is_eos = true;
    } else {
      eosi = 0;
      is_eos = false;
    }

    // -- eos run-up / -down
    for ( ; eosi >= 0; eosi--) {
      token = (char *)lexer.yytext;
      train_next_token();
    }
  }
  return cleanup_training_temps();
}

/*
 * bool dwds_tagger_trainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
 */
bool dwds_tagger_trainer::train_from_strings(int argc, char **argv, FILE *out=stdout)
{
  // -- sanity check
  if (!can_tag() || !init_training_temps(NULL,out)) {
    fprintf(stderr, "dwds_tagger_trainer::train_from_strings(): cannot run uninitialized trainer!\n");
    return false;
  }

  // -- ye olde guttes
  for ( ; --argc >= 0; argv++) {
    token = *argv;
    train_next_token();
  }
  // -- eos rundown
  is_eos = true;
  while (++argc <= kmax) {
    train_next_token();
  }
  return cleanup_training_temps();
}


/*
 * bool dwds_tagger_trainer::init_training_temps()
 */
inline bool dwds_tagger_trainer::init_training_temps(FILE *in=NULL, FILE *out=NULL)
{
  int i;

  // -- ngram temps
  curngrams = new set<NGramVector>();
  nextngrams = new set<NGramVector>();

  // -- files
  infile = in;
  outfile = out;

  // -- initialize string-sets
  for (i = 0; i < kmax; i++) {
    curtags = new set<FSMSymbolString>();
    curtags->insert(eos);
    stringq.push_back(curtags);
  }
  return true;
}


/*
 * bool dwds_tagger_trainer::cleanup_training_temps()
 */
inline bool dwds_tagger_trainer::cleanup_training_temps()
{
  int i;

  // -- cleanup string-sets
  for (i = 0; i < kmax; i++) {
    curtags = stringq.front();
    stringq.pop_front();
    if (curtags) {
      curtags->clear();
      delete curtags;
    }
  }
  // -- cleanup ngram-temps
  if (curngrams) {
    curngrams->clear();
    delete curngrams;
    curngrams = NULL;
  }
  if (nextngrams) {
    nextngrams->clear();
    delete nextngrams;
    nextngrams = NULL;
  }
  tmpngrams = NULL;

  infile = NULL;
  outfile = NULL;
  return true;
}

/*
 * inline void dwds_tagger_trainer::train_next_token(void);
 *   + does the whahosis for the next token
 */
inline void dwds_tagger_trainer::train_next_token(void)
{
    // -- tag it first
    if (!is_eos) {
      s = (char *)token;
      tmp->fsm_clear();
      result = morph->fsm_lookup(s,tmp,true);
      // -- verbosity
      if (verbose) ntokens++;
    }

    // -- pop 'current' tag-string set (last in queue == oldest)
    curtags = stringq.back();
    stringq.pop_back();

    // -- get the 'next' tag-string set to the 'current' one
    curtags->clear();
    if (!is_eos) {
      if (want_features) {
	// -- all features
	results.clear();
	tmp->fsm_strings(*syms, results, false, want_avm);

	// -- set curtags to full string results
	for (ri = results.begin(); ri != results.end(); ri++) {
	  curtags->insert(ri->istr);
	}
      } else {
	// -- tags only
	get_fsm_tag_strings(tmp,curtags);
	/*for (cti = curtags->begin(); cti != curtags->end(); cti++) {
	  // -- convert to a valid regex
	  FSMSymbolString hacked_tag = *cti;
	  if (want_avm && hacked_tag[0] == '_') {
	    hacked_tag.erase(0,1);
	  }
	  if (hacked_tag[0] != '_') { hacked_tag.insert(0,"["); }
	  if (hacked_tag[hacked_tag.length()] != ']') { hacked_tag.append("]"); }
	  curtags->erase(cti);
	  curtags->insert(hacked_tag);
	  }*/
      }
    } else {
      // -- terminal EOS
      curtags->insert(eos);
    }
    
    // -- unknown token?
    if (curtags->empty()) {
      get_fsm_tag_strings(ufsa,curtags);
      // -- HACK: make it look like an AVM struct
      /*for (cti = curtags->begin(); cti != curtags->end(); cti++) {
	FSMSymbolString hacked_tag = *cti;
	if (want_avm && hacked_tag[0] == '_') {
	  hacked_tag.erase(0,1);
	}
	if (hacked_tag[0] != '[') { hacked_tag.insert(0,"["); }
	if (hacked_tag[hacked_tag.length()] != ']') { hacked_tag.append("]"); }
	curtags->erase(cti);
	curtags->insert(hacked_tag);
	}*/
    }
    
    // -- push 'current' tags onto the queue (front of queue == newest)
    stringq.push_front(curtags);
    
    // -- note all 'current' tags in "alltags"
    alltags.insert(curtags->begin(),curtags->end());

    // ----------------------------
    // counting: i <= kmax -grams
    // ----------------------------
    curngrams->clear();
    theNgram.clear();
    for (qi = stringq.rbegin(); qi != stringq.rend(); qi++) {
      if (curngrams->empty()) {
	// -- initialize current-ngrams (when qi==stringq.rbegin())
	//    : curngrams are initialized to oldest string-set
	for (qii = (*qi)->begin(); qii != (*qi)->end(); qii++) {
	  curngrams->insert(NGramVector(1,*qii));
	}
      } else {
	// -- we already have some ngrams -- need to extend them
	nextngrams->clear();
	for (ngi = curngrams->begin(); ngi != curngrams->end(); ngi++) {
	  for (qii = (*qi)->begin(); qii != (*qi)->end(); qii++) {
	    theNgram = *ngi;
	    theNgram.push_back(*qii);
	    nextngrams->insert(theNgram);
	  }
	}
	// -- swap curngrams and nextngrams
	tmpngrams = curngrams;
	curngrams = nextngrams;
	nextngrams = tmpngrams;
      }
      // -- ... and count them
      for (ngi = curngrams->begin(); ngi != curngrams->end(); ngi++) {
	theNgram = *ngi;
	if (ngtable.find(theNgram) != ngtable.end()) {
	    ngtable[theNgram] += curngrams->size() >= 0 ? 1.0/(float)curngrams->size() : 0;
	} else {
	    ngtable[theNgram] = curngrams->size() >= 0 ? 1.0/(float)curngrams->size() : 0;
	}
      }
    }
}


/*--------------------------------------------------------------------------
 * Public Methods: Parameter generation: Parameter-File Read/Write
 *--------------------------------------------------------------------------*/

/*
 * bool dwds_tagger_trainer::write_param_file(FILE *out=stdout);
 */
bool dwds_tagger_trainer::write_param_file(FILE *out=stdout)
{
  // -- use a sorted string-list
  //set<FSMSymbolString> allstrings;
  //for (sci = strings2counts.begin(); sci != strings2counts.end(); sci++) {
  //  allstrings.insert(sci->first);
  //}
  //for (set<FSMSymbolString>::iterator asi = allstrings.begin(); asi != allstrings.end(); asi++) {
  //  fprintf(out, "%s\t%f\n", asi->c_str(), strings2counts[*asi]);
  //}

  // -- unsorted
  for (ngti = ngtable.begin(); ngti != ngtable.end(); ngti++) {
    for (NGramVector::const_iterator ngvi = ngti->first.begin(); ngvi != ngti->first.end(); ngvi++) {
      fputs(ngvi->c_str(),out);
      fputc('\t',out);
    }
    fprintf(out,"%f\n",ngti->second);
  }
  return true;
}

/*
 * read_param_file(file,filename)
 *  + adds counts from given param file to internal table 'strings2counts'
 */
bool dwds_tagger_trainer::read_param_file(FILE *in=stdin,const char *filename=NULL)
{
  //--  MECKER
  fprintf(stderr,"dwdst_tagger_trainer::read_param_file(): not yet implemented!\n");
  abort();
  return false;

  /*
  int tok;
  dwdst_param_lexer plexer;
  plexer.select_streams(in,stdout);

  FSMSymbolString symstr;
  float symcnt;

  while ((tok = plexer.yylex()) != dwdst_param_lexer.PF_EOF) {
    switch (tok) {
    case dwdst_param_lexer::PF_REGEX:
      symstr.append((char *)lexer.yytext);
      break;
    case dwdst_param_lexer::PF_COUNT:
      symcnt = atof((char *)lexer.yytext);
      break;
    default:
    }
    }*/
  return true;
}

/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------*/
