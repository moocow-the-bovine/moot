/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: dwdstCHMM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : Class-based Hidden Markov Model: headers
 *--------------------------------------------------------------------------*/

#ifndef _DWDST_CHMM_H
#define _DWDST_CHMM_H

#include "dwdstMorph.h"
#include "dwdstHMM.h"
#include "dwdstTypes.h"
#include "dwdstBinStream.h"

#include <zlib.h>

#include <FSMCost.h>

/*--------------------------------------------------------------------------
 * dwdstCHMM : HMM class with support for lexical classes (morphology output)
 *--------------------------------------------------------------------------*/

/**
 * \brief 1st-order Hidden Markov Model Tagger/Disambiguator class
 * with built-in support for lexical classes (morphology output).
 */
class dwdstCHMM : public dwdstHMM {
public:
  /*------------------------------------------------------------
   * public typedefs : lexical classes
   */

  /** Type for a lexical-class */
  typedef set<TagID> LexClass;

  /** Utility struct for hash_map<>. */
  struct LexClassHash {
  public:
    inline size_t operator()(const LexClass &x) const {
      size_t hv = 0;
      for (LexClass::const_iterator xi = x.begin(); xi != x.end(); xi++) {
	hv = 5*hv + *xi;
      }
      return hv;
    };
  };
  /** Utility struct for hash_map<>. */
  struct LexClassEqual {
  public:
    inline size_t operator()(const LexClass &x, const LexClass &y) const {
      return x==y;
    };
  };

  /** Typedef for a lexical ClassID */
  typedef dwdstEnumID ClassID;

  /** ClassID lookup table */
  typedef
    dwdstEnum<LexClass,
	      LexClassHash,
	      LexClassEqual>
    ClassIDTable;


  /** Type for lexical-class probability lookup subtable: tagid->prob */
  typedef map<TagID,ProbT> LexClassProbSubTable;

  /** Type for lexical-class probability lookup table: classid->(tagid->p(classid|tagid)) */
  typedef vector<LexClassProbSubTable> LexClassProbTable;



  /*------------------------------------------------------------
   * public typedefs : morphology
   */

  /** Single morphological analysis, including features */
  typedef dwdstMorph::MorphAnalysis MorphAnalysis;

  /** Set of morphological analyses, including features */
  typedef dwdstMorph::MorphAnalysisSet MorphAnalysisSet;

  /** Single morphological analysis, with tag and weight extracted */
  typedef FSMSymbolString TagMorphAnalysis;

  /**
   * Set of morphological analyses, with tag and weight extracted,
   * This class is here in order to:
   * (1) minimize copying by paying attention to analysis-weights, and
   * (2) convert morphological analyses to strings
   */
  //typedef set<TagMorphAnalysis> TagMorphAnalysisSet;
  class TagMorphAnalysisSet {
  public:
    set<TagMorphAnalysis> analyses;  /**< Morphological analyses */
    FSMWeight               weight;  /**< Weight of these analyses */
  public:
    /** Default constructor */
    TagMorphAnalysisSet(FSMWeight w=MAXFLOAT)
      : weight(w)
    {};

    /** Destructor */
    ~TagMorphAnalysisSet(void) {};

    /** Insertion: strings: weight-watcher */
    inline void insert(const TagMorphAnalysis &a, const FSMWeight w)
    {
      if (w > weight) return; //-- too costly: ignore
      if (w < weight) {
	//-- cheaper: clear and insert
	analyses.clear();
	weight = w;
      }
      //-- equal: just insert
      analyses.insert(a);
    };

    /** Insertion: weighted strings */
    inline void insert(const FSM::FSMStringWeight &sw)
    {
      insert(sw.istr, sw.weight);
    };

    /** Insertion: vectors: weight-watcher */
    inline void insert(const FSM::FSMSymbolVector &av, const FSMWeight w, const dwdstMorph &morph)
    {
      if (w > weight) return; //-- too costly: ignore

      //-- stringify
      TagMorphAnalysis as;
      morph.symbol_vector_to_string(av, as);

      if (w < weight) {
	//-- cheaper: clear and insert
	analyses.clear();
	weight = w;
      }
      //-- equal: just insert
      analyses.insert(as);
    };

    /** Insertion: weighted vectors */
    inline void insert(const FSM::FSMWeightedSymbolVector &wv, const dwdstMorph &morph)
    {
      insert(wv.istr, wv.weight, morph);
    };
  };

  /*------------------------------------------------------------
   * public typedefs : runtime data
   */

  /** Type for intermediate token-information: map tags to morphological analysis-sets */
  typedef
    map<TagID,TagMorphAnalysisSet>
    TagMorphMap;

  /*------------------------------------------------------------
   * public typedefs : caching
   */

  /** Type for morphologica-map cache: tokid->TagMorphMap */
  typedef hash_map<TokID,TagMorphMap> TokMorphCache;

public:
  /*------------------------------------------------------------
   * public data : flags
   */
  bool want_tags_only;  /* Whether to output only tags (no big speed benefits at the moment) */

  /*------------------------------------------------------------
   * public data : morphology
   */
  dwdstMorph        morph;          /**< Morphological analyzer */

  /*------------------------------------------------------------
   * public data : lookup table(s)
   */
  size_t            n_classes;    /**< Number of known classes */
  ClassIDTable      classids;     /**< Class-ID lookup table */
  LexClassProbTable lcprobs;      /**< Lexical-class probability lookup table */


  /*------------------------------------------------------------
   * public data : smoothing & other constant(s)
   */
  ProbT morph_cache_threshhold;   /**< Morphological cache threshhold */

  /*------------------------------------------------------------
   * public data : cache(s)
   *   + For 'morphcache', index 0 corresponds to
   *     the "unknown" lexical class: a failed morphological analysis:
   *     this data gets generated at compile-time.
   */
  TokMorphCache    morphcache;       /**< Morphology cache */
  ClassID          uclassid;         /**< Class-ID for tokens unknown to morphology */

  /*------------------------------------------------------------
   * public data : per-token saved information
   */
  vector<TagMorphMap> tmmaps;  /**< Temporarily stores per-token tagid->MorphAnalysisSet maps */

protected:

  /*------------------------------------------------------------
   * protected data : temporaries
   */
  FSMSymbolString symstr_tmp;   /**< Temporary string for conversions */
  LexClass        lexclass_tmp; /**< Temporary lexical class for conversions / lookup */

public:

  /*------------------------------------------------------------
   * public methods: constructor / destructor
   */
  /** Default constructor */
  dwdstCHMM(void) : 
    want_tags_only(false),
    uclassid(0)
  {};

  /** Destructor */
  virtual ~dwdstCHMM(void)
  {
    clear();
  };

  /*------------------------------------------------------------
   * public methods: load morphology
   */
  inline bool load_morphology(const char *symbolfile, const char *fstfile)
  {
    return (morph.load_morph_symbols(symbolfile) && morph.load_morph_fst(fstfile));
  };

  /*------------------------------------------------------------
   * public methods: reset/clear
   */

  /**
   * Reset/clear the object, freeing all dynamic data structures.
   * If 'wipe_everything' is false, ID-tables and constants will
   * spared.
   */
  void clear(bool wipe_everything=true);



  /*------------------------------------------------------------
   * public methods: ClassID lookup : from LexClass
   */

  /**
   * Lookup the ClassID for the lexical-class 'lclass'.
   * A new class-ID will be generated if 'lclass' is not currently defined.
   * Additionally, if 'autopopulate' is true (the default), the new class
   * will be populated with default (equal probability) values.
   */
  inline ClassID class2classid(const LexClass &lclass, bool autogen=true) {
    //-- table lookup
    ClassID cid = classids.name2id(lclass);
    if (cid == 0) {
      //-- previously unknown class: fill 'er up with default values
      cid = classids.insert(lclass);
      if (cid >= lcprobs.size()) {
	n_classes = cid+1;
	lcprobs.resize(n_classes);
      }
      if (autogen) {
	LexClassProbSubTable &lcps = lcprobs[cid];
	if (!lclass.empty()) {
	  //-- non-empty class: restrict population to class-members
	  ProbT lcsize = (ProbT)lclass.size();
	  for (LexClass::const_iterator lci = lclass.begin(); lci != lclass.end(); lci++) {
	    lcps[*lci] = 1.0/lcsize;
	  }
	} else {
	  //-- empty class: use class for "unknown" token instead [HACK!]
	  const LexProbSubTable &lps = lexprobs[0];
	  ProbT lpsize = (ProbT)lps.size();
	  for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++) {
	    lcps[lpsi->first] = 1.0/lpsize;
	  }
	}
      }
    }
    return cid;
  };


  /*------------------------------------------------------------
   * public methods: TagMorphMap generation / lookup
   */

  /**
   * Looks up (possibly empty) TagID->MorphAnalysisSet map for 'token'.  Uses
   * cached information if available.
   */
  inline void token2morphmap(const dwdstTokString &token, TagMorphMap &tmm, TokID tokid=0)
  {
    //-- "unknown" token check : must call morphology
    if (tokid==0) {
      get_token_morphmap(token,tmm);
      return;
    }
    //-- check the cache
    TokMorphCache::const_iterator tmci = morphcache.find(tokid);
    if (tmci != morphcache.end()) {
      tmm = tmci->second;
      return;
    }
    //-- when all else fails, call morphology
    get_token_morphmap(token,tmm);
  };

  /**
   * Builds (possibly empty) TagID->MorphAnalysisSet map for 'token' by morphological analysis.
   * No cached information is used.
   */
  inline void get_token_morphmap(const dwdstTokString &token, TagMorphMap &tmm)
  {
    analyses2morphmap(morph.tag_token(token), tmm);
  };

  /** Convert a raw morphogical analysis-set to a TagMorphMap */
  inline const TagMorphMap &analyses2morphmap(const MorphAnalysisSet &ma, TagMorphMap &tmm)
  {
    tmm.clear();
    for (MorphAnalysisSet::const_iterator mai = ma.begin(); mai != ma.end(); mai++) {
      TagID tagid = vector2tagid(*mai);
      if (tagid == 0) continue; //-- ignore the "unknown" tag
      tmm[tagid].insert(*mai, morph);
    }
    return tmm;
  };

  /*------------------------------------------------------------
   * public methods: ClassID lookup : from TagMorphMap
   */

  /**
   * Get the ClassID corresponding to the set of tags whose IDs are keys
   * of 'tmm'. No cache is used.
   */
  inline ClassID morphmap2classid(const TagMorphMap &tmm)
  {
    //-- generate class-name, then generate class if necessary
    return get_morphmap_classid(tmm);
  };

  /**
   * Get the ClassID corresponding to the set of tags whose IDs are keys
   * of 'tmm'.  No cached information is used.
   */
  inline ClassID get_morphmap_classid(const TagMorphMap &tmm)
  {
    return class2classid(get_morphmap_class(tmm));
  };

  /** Convert a TagMorphMap to a LexClass, no caches involved. */
  inline LexClass &get_morphmap_class(const TagMorphMap &tmm)
  {
    lexclass_tmp.clear();
    for (TagMorphMap::const_iterator tmmi = tmm.begin(); tmmi != tmm.end(); tmmi++) {
      lexclass_tmp.insert(tmmi->first);
    }
    return lexclass_tmp;
  };


  /*------------------------------------------------------------
   * public methods: Vector->TagID conversion
   */

  /**
   * Returns the tag-ID for the FSMSymbolVector 'vec'.
   */ 
  inline TagID vector2tagid(const FSM::FSMSymbolVector &vec)
  {
    symstr_tmp.clear();
    morph.symbol_vector_to_string(vec,symstr_tmp);
    return tagids.name2id(symstr_tmp);
  };

  /**
   * Returns the tag-ID for the morphological analysis (weighted vector pair) 'ma'.
   */ 
  inline TagID vector2tagid(const MorphAnalysis &ma)
  {
    return ma.ostr.empty()
      ? vector2tagid(ma.istr)
      : vector2tagid(ma.ostr);
  };


  /*------------------------------------------------------------
   * public methods: compilation
   */

  /**
   * Compile probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  bool compile(const dwdstLexfreqs &lexfreqs,
	       const dwdstNgrams &ngrams,
	       const dwdstTagString &start_tag_str=dwdstTagString("__$"));


  /** Compile data for empty lexical class: called by compile(). */
  bool compile_unknown_lclass(void);

  /*------------------------------------------------------------
   * public methods: high-level: Viterbi: clear state tables
   */

  /** Clear Viterbi state table(s) */
  inline void viterbi_clear(void)
  {
    //-- superclass clear
    dwdstHMM::viterbi_clear();

    //-- clear morphmaps
    tmmaps.clear();
  };


  /*------------------------------------------------------------
   * public methods: high-level: Viterbi: single iteration: (tokstr)
   */

  /**
   * Step a single Viterbi iteration.
   */
  inline void viterbi_step(const dwdstTokString &tokstr)
  {
    //-- Get Token-ID
    TokID tokid = token2id(tokstr);

    //-- Get morphological map
    tmmaps.push_back(TagMorphMap());
    TagMorphMap &tmm = tmmaps.back();
    token2morphmap(tokstr, tmm, tokid);
    ClassID classid = 0;
    if (tmm.empty()) {
      //-- unknown to morphology?
      tmm     = morphcache[0];
      classid = uclassid;
    } else if (tokid==0) {
      //-- Get class-ID and class probability table (HMM-unknown tokens only!)
      classid = morphmap2classid(tmm);
    }

    //-- ye olde guttes
    _viterbi_step_sub(tokid, classid, tmm);
  };


  /**
   * Step a single Viterbi iteration, given a TokID and a MorphAnalysisSet
   */
  inline void viterbi_step(const TokID tokid, const MorphAnalysisSet &ma)
  {
    //-- Get morphological map
    tmmaps.push_back(TagMorphMap());
    TagMorphMap &tmm = tmmaps.back();
    ClassID classid  = 0;
    if (ma.empty()) {
      //-- unknown to morphology
      tmm     = morphcache[0];
      classid = uclassid;
    } else {
      //-- known to morphology: get morphmap
      analyses2morphmap(ma, tmm);
      if (tokid == 0) {
	//-- lookup classid (HMM-unknown tokens only!)
	classid = morphmap2classid(tmm);
      }
    }

    //-- ye olde guttes
    _viterbi_step_sub(tokid, classid, tmm);
  };


  /**
   * Low-level: Step a single Viterbi iteration: 'tmm' is NOT added to 'tmmaps' (that
   * should happen at a higher level).
   */
  inline void _viterbi_step_sub(TokID tokid, ClassID classid, const TagMorphMap &tmm)
  {
    //-- Get next column
    ViterbiColumn *col = viterbi_get_column();
    ViterbiNode   *nod;
    col->col_prev = vtable;
    col->nodes = NULL;

    //-- Get Token & Class probability table(s)
    const LexProbSubTable      &lps  = lexprobs[tokid];
    const LexClassProbSubTable &lcps = lcprobs[classid];

    //-- for each possible destination tag 'vtagid'
    LexProbSubTable::const_iterator      lpsi;
    LexClassProbSubTable::const_iterator lcpsi;
    for (TagMorphMap::const_iterator tmmi = tmm.begin(); tmmi != tmm.end(); tmmi++) {
      vtagid = tmmi->first;
      
      //-- ignore "unknown" tag
      if (vtagid == 0) continue;
      
      //-- get lexical probability
      if (tokid != 0) {
	//-- Known token: use "real" lexical probabilities
	//-- P(tok|tag) := lambda_{w1}*p(tok|tag) + lambda_{w2}
	lpsi = lps.find(vtagid);
	vwordpr = (wlambda2 + (lpsi  == lps.end()  ? 0 : (wlambda1 * lpsi->second)) );
      }
      else if (classid != uclassid && classid != 0) {
	//-- Unknown token, known class: use class probabilities
	//-- P(tok|tag) := lambda_{c}*p(class|tag) + lambda_{w2}
	lcpsi = lcps.find(vtagid);
	vwordpr = (wlambda2 + (lcpsi  == lcps.end()  ? 0 : (wlambda1 * lcpsi->second)) );
      } else { // (classid == uclassid || classid == 0)
	//-- Totally unknown token: evenly weight "unknown" token and "unknown" class
	//   P(tok|tag) := 0.5*lambda_{w1}*p(tok|tag) + 0.5*lambda_{c}*p(class|tag) + lambda_{w2}
	lpsi = lps.find(vtagid);
	lcpsi = lcps.find(vtagid);
	vwordpr = (wlambda2
		   + (lpsi  == lps.end()  ? 0 : (0.5 * wlambda1 * lpsi->second))
		   + (lcpsi == lcps.end() ? 0 : (0.5 * wlambda1 * lcpsi->second)) );
      }

      
      //-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
      viterbi_find_best_prevnode(vtagid, tagp(vtagid));
      
      //-- update state table column for current destination tag
      nod           = viterbi_get_node();
      nod->tagid    = vtagid;
      nod->prob     = vbestpr * vwordpr;
      nod->pth_prev = vbestpn;
      nod->row_next = col->nodes;
      col->nodes    = nod;
    }
    
    //-- add new column to state table
    vtable = col;
  };


  /*------------------------------------------------------------
   * public methods: top-level
   */

  /** Top-level tagging interface: tag tokens from a C-stream using dwdstTaggerLexer */
  bool tag_stream(FILE *in=stdin, FILE *out=stdout, char *srcname=NULL);

  /** Top-level tagging interface: string input (array of tokens) */
  void tag_strings(int argc, char **argv, FILE *out=stdout, char *infilename=NULL);

  /** Top-level tagging interface: dump best path */
  inline void tag_print_best_path(FILE *out=stdout)
  {
    viterbi_best_path(); //-- populate 'vbestpath' with (ViterbiNode*)s
    for (size_t i = 0; i < tokens.size(); i++) {
      if (i+1 >= vbestpath.size()) {
	carp("%s: Error: no tag for token number %u of sentence number %u: '%s'\n",
	     i, nsents, tokens[i].c_str());
	continue;
      }
      else if (i >= tmmaps.size()) {
	carp("%s: Error: no TagMorphMap for token number %u of sentence number %u: '%s'\n",
	     i, nsents, tokens[i].c_str());
	continue;
      }
      TagID tagid = vbestpath[i+1]->tagid;
      print_token_analyses(out,
			   tokens[i],
			   tagids.id2name(tagid),
			   tmmaps[i][tagid].analyses);
    }
    morph.tag_print_eos(out);
  };

  /*------------------------------------------------------------
   * public methods: high-level: tagging: output
   */
  inline void print_token_analyses(FILE *out,
				   const dwdstTokString &token, 
				   const dwdstTagString &tagstr,
				   const set<TagMorphAnalysis> &tms)
  {
    fputs(token.c_str(), out);
    set<TagMorphAnalysis>::const_iterator tmsi;
    if (morph.want_mabbaw_format) {
      /*-- ambiguous, strings, all features, mabbaw-style */
      fprintf(out, ": %d Analyse(n)\n", tms.size());
      if (!want_tags_only) {
	//-- Print all analyses
	for (tmsi = tms.begin(); tmsi != tms.end(); tmsi++) {
	  fputc('\t', out);
	  //-- Print the tag first
	  fputc('\t', out);
	  fputs(tagstr.c_str(), out);
	  fputc(':', out); 
	  fputs(tmsi->c_str(), out);
	  fputc('\n', out);
	}
      }
      fputc('\n', out);
    } else {
      /*-- ambiguous, strings, all features, one tok/line */
      //-- Print the tag first
      fputc('\t', out);
      fputs(tagstr.c_str(), out);
      if (!want_tags_only) {
	//-- Print all analyses
	for (tmsi = tms.begin(); tmsi != tms.end(); tmsi++) {
	  fputc('\t', out);
	  fputs(tmsi->c_str(), out);
	}
      }
      fputc('\n', out);
    }
  };

  /*------------------------------------------------------------
   * public methods: high-level: binary load/save
   */

  /** Save to a binary file */
  bool save(const char *filename, int compression_level=Z_DEFAULT_COMPRESSION);

  /** Save to a binary stream */
  bool save(dwdstBinStream::oBinStream &obs, const char *filename=NULL);

  /** Low-level: save guts to a binary stream */
  bool _bindump(dwdstBinStream::oBinStream &obs, const char *filename=NULL);

  /** Load from a binary file */
  bool load(const char *filename=NULL);

  /** Load from a binary stream */
  bool load(dwdstBinStream::iBinStream &ibs, const char *filename=NULL);

  /** Low-level: load guts from a binary stream */
  bool _binload(dwdstBinStream::iBinStream &ibs, const char *filename=NULL);


  /*------------------------------------------------------------
   * public methods: low-level: debugging
   */

  /** Debugging method: dump basic CHMM contents to a text file. */
  void txtdump(FILE *file);

  /* Debugging method: dump current Viterbi state-table column to a text file */
  //void viterbi_txtdump(FILE *file);
};


#endif /* _DWDST_CHMM_H */
