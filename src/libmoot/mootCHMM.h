/* -*- Mode: C++ -*- */
/*--------------------------------------------------------------------------
 * File: mootCHMM.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description:
 *   + PoS tagger for DWDS project : Class-based Hidden Markov Model: headers
 *--------------------------------------------------------------------------*/

#ifndef _moot_CHMM_H
#define _moot_CHMM_H

#include "mootMorph.h"
#include "mootHMM.h"
#include "mootTypes.h"
#include "mootBinStream.h"

#include <zlib.h>

#include <FSMCost.h>

moot_BEGIN_NAMESPACE

/*--------------------------------------------------------------------------
 * mootCHMM : HMM class with support for lexical classes (morphology output)
 *--------------------------------------------------------------------------*/

/**
 * \brief 1st-order Hidden Markov Model Tagger/Disambiguator class
 * with built-in support for lexical classes (morphology output).
 */
class mootCHMM : public mootHMM {
public:
  /*------------------------------------------------------------
   * public typedefs : lexical classes
   */
  /// \name Lexical class types
  //@{

  /** Type for a lexical-class. */
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

  /**
   * Typedef for a lexical ClassID. Zero indicates a previously
   * unknown class.  If you want the ClassID associated with
   * unknown tokens, use tokids2classids[0], which ought to
   * give you a "real" ClassID.
   */
  typedef mootEnumID ClassID;

  /** ClassID lookup table */
  typedef
    mootEnum<LexClass,
	      LexClassHash,
	      LexClassEqual>
    ClassIDTable;


  /** Type for lexical-class probability lookup subtable: tagid->prob */
  typedef map<TagID,ProbT> LexClassProbSubTable;

  /** Type for lexical-class probability lookup table: classid->(tagid->p(classid|tagid)) */
  typedef vector<LexClassProbSubTable> LexClassProbTable;
  //@}


  /*------------------------------------------------------------
   * public typedefs : morphology
   */
  /// \name Morphology types
  //@{

  /** Single morphological analysis, including features */
  typedef mootMorph::MorphAnalysis MorphAnalysis;

  /** Set of morphological analyses, including features */
  typedef mootMorph::MorphAnalysisSet MorphAnalysisSet;

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
    inline void insert(const FSM::FSMSymbolVector &av, const FSMWeight w, const mootMorph &morph)
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
    inline void insert(const FSM::FSMWeightedSymbolVector &wv, const mootMorph &morph)
    {
      insert(wv.istr, wv.weight, morph);
    };
  };
  //@}

  /*------------------------------------------------------------
   * public typedefs : runtime data
   */
  /// \name Runtime & API types
  //@{

  /** Type for intermediate token-information: map tags to morphological analysis-sets */
  typedef map<TagID,TagMorphAnalysisSet> TagMorphMap;

  /** Type for temporary per-token morphological information */
  struct TmpTagMorphMap {
    TagMorphMap     tmmap;
    TmpTagMorphMap *next;
  };
  //@}


  /*------------------------------------------------------------
   * public typedefs : caching
   */
  /// \name Cache Types
  //@{

  /** Type for morphological-map cache: tokid->TagMorphMap */
  typedef hash_map<TokID,TagMorphMap> TokMorphCache;
  //@}

  /*------------------------------------------------------------
   * public typedefs : Viterbi
   */
  /// \name Viterbi State-Table types
  //@{

  /** Type for a Viterbi state-table entry (linked-list columns) */
  class ViterbiNode : public mootHMM::ViterbiNode
  {
  public:
    TagMorphAnalysisSet   *mas;     /**< New: Morphological analyses for this node only */
  };
  //@}

public:
  /*------------------------------------------------------------
   * public data : flags
   */
  /// \name flags
  //@{
  bool want_pos_only;  /* Whether to output only PoS-tags (no big speed benefits at the moment) */
  //@}

  /*------------------------------------------------------------
   * public data : morphology
   */
  /// \name Morphology
  //@{
  mootMorph        morph;          /**< Morphological analyzer */
  //@}

  /*------------------------------------------------------------
   * public data : lookup table(s)
   */
  /// \name Lookup tables
  //@{
  size_t            n_classes;    /**< Number of known classes */
  ClassIDTable      classids;     /**< Class-ID lookup table */
  LexClassProbTable lcprobs;      /**< Lexical-class probability lookup table */
  //@}

  /*------------------------------------------------------------
   * public data : smoothing & other constant(s)
   */
  /// \name Constants
  //@{
  ProbT       morph_cache_threshhold;   /**< Morphological cache threshhold */
  ClassID     uclassid;                 /**< Class-ID for tokens unknown to morphology */
  //@}

  /*------------------------------------------------------------
   * public data : cache(s)
   *   + For 'morphcache', index 0 corresponds to
   *     the "unknown" lexical class: a failed morphological analysis:
   *     this data gets generated at compile-time.
   */
  /// \name Caches
  //@{
  TokMorphCache    morphcache;       /**< Morphology cache */
  TagMorphMap    tmm_fallback;       /**< Fallback morphological map */
  //@}

  /*------------------------------------------------------------
   * public data : per-token saved information
   */
  //(none)

protected:

  /*------------------------------------------------------------
   * protected data : temporaries
   */
  /// \name Pre-allocated temporaries
  //@{
  FSMSymbolString   symstr_tmp;   /**< Temporary string for conversions */
  LexClass          lexclass_tmp; /**< Temporary lexical class for conversions / lookup */

  /*------------------------------------------------------------
   * protected data : temporary (TagMorphMap)s
   */
  TmpTagMorphMap *ttmms_used;  /**< (TmpTagMorphMap)s currently in use */
  TmpTagMorphMap *ttmms_free;  /**< (TmpTagMorphMap)s currently not in use */
  //@}

public:

  /*------------------------------------------------------------
   * public methods: constructor / destructor
   */
  /// \name Constructor / destructor
  //@{
  /** Default constructor */
  mootCHMM(void) : 
    want_pos_only(false),
    uclassid(0),
    ttmms_used(NULL),
    ttmms_free(NULL)
  {};

  /** Destructor */
  virtual ~mootCHMM(void)
  {
    clear(true);
  };
  //@}

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
   * public methods: high-level: binary load/save
   */
  /// \name Binary load/save
  //@{
  /** Save to a binary file */
  bool save(const char *filename, int compression_level=Z_DEFAULT_COMPRESSION);

  /** Save to a binary stream */
  bool save(mootBinStream::oBinStream &obs, const char *filename=NULL);

  /** Low-level: save guts to a binary stream */
  bool _bindump(mootBinStream::oBinStream &obs, const char *filename=NULL);

  /** Load from a binary file */
  bool load(const char *filename=NULL);

  /** Load from a binary stream */
  bool load(mootBinStream::iBinStream &ibs, const char *filename=NULL);

  /** Low-level: load guts from a binary stream */
  bool _binload(mootBinStream::iBinStream &ibs, const char *filename=NULL);
  //@}

  /*------------------------------------------------------------
   * public methods: load morphology
   */
  /// \name Initialization / Compilation
  //@{
  inline bool load_morphology(const char *symbolfile, const char *fstfile)
  {
    return (morph.load_morph_symbols(symbolfile) && morph.load_morph_fst(fstfile));
  };

  /*------------------------------------------------------------
   * public methods: compilation
   */

  /**
   * Compile probabilites from raw frequency counts in 'lexfreqs' and 'ngrams'.
   * Returns false on failure.
   */
  bool compile(const mootLexfreqs &lexfreqs,
	       const mootNgrams &ngrams,
	       const mootTagString &start_tag_str=mootTagString("__$"));


  /** Compile data for empty lexical class: called by compile(). */
  bool compile_unknown_lclass(void);
  //@}

  /*------------------------------------------------------------
   * public methods: high-level: Viterbi: clear state tables
   */

  /** \name High-level Viterbi algorithm API */
  //@{

  /** Clear Viterbi state table(s) */
  inline void viterbi_clear(void)
  {
    //-- superclass clear
    mootHMM::viterbi_clear();

    //-- unmark used (TagMorphMap)s
    TmpTagMorphMap *ttmm, *ttmm_next;
    for (ttmm = ttmms_used; ttmm != NULL; ttmm = ttmm_next) {
      ttmm_next   = ttmm->next;
      ttmm->next  = ttmms_free;
      ttmms_free  = ttmm;
    }
    ttmms_used = NULL;
  };


  /*------------------------------------------------------------
   * public methods: high-level: Viterbi: single iteration: (tokstr)
   */

  /**
   * Step a single Viterbi iteration.
   */
  inline void viterbi_step(const mootTokString &tokstr)
  {
    //-- Get Token-ID
    TokID tokid = token2id(tokstr);

    //-- Get morphological map
    TagMorphMap  *tmm = token2morphmap(tokstr, tokid);
    ClassID classid   = 0;
    if (tmm->empty()) {
      //-- unknown to morphology?
      tmm     = &(morphcache[0]);
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
    //-- Get a temporary morphological map
    TagMorphMap *tmm = get_tmp_morphmap();
    ClassID  classid = 0;
    if (ma.empty()) {
      //-- unknown to morphology
      tmm     = &(morphcache[0]);
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

  //------------------------------------------------------------
  // public methods: high-level: Viterbi: finish

  /**
   * Run final Viterbi iteration, using 'final_tagid' as the final tag
   */
  inline void viterbi_finish(const TagID final_tagid)
  {
    _viterbi_push(final_tagid);
  };

  /**
   * Run final Viterbi iteration, using instance datum 'start_tagid' as the final tag.
   */
  inline void viterbi_finish(void)
  {
    _viterbi_push(start_tagid);
  };
  //@}

  /*------------------------------------------------------------
   * public methods: top-level
   */
  /// \name Top-level tagging interface 
  //@{

  /** Tag tokens from a C-stream using mootTaggerLexer */
  bool tag_stream(FILE *in=stdin, FILE *out=stdout, char *srcname=NULL);

  /** Tag a C-array of token-strings (as a sentence) */
  void tag_strings(int argc, char **argv, FILE *out=stdout, char *infilename=NULL);
  //@}

  /// \name Tagging interface output
  //@{

  /** Dump best path for current sentence buffer. */
  inline void tag_print_best_path(FILE *out=stdout)
  {
    //-- populate 'vbestpath' with (ViterbiPathNode*)s
    ViterbiPathNode *pnod = viterbi_best_path();
    vector<mootTokString>::const_iterator toki;

    if (pnod) pnod = pnod->path_next;

    for (toki = tokens.begin(); toki != tokens.end(); toki++) {
      if (pnod && pnod->node) {
	print_token_analyses(out,
			     *toki,
			     tagids.id2name(pnod->node->tagid),
			     ((ViterbiNode *)(pnod->node))->mas);
	pnod = pnod->path_next;
      }
      else {
	//-- this should never actually happen, but it has...
	carp("%s: Error: no best tag for token '%s'!\n",
	     "mootCHMM::tag_print_best_path()", toki->c_str());
	print_token_analyses(out, *toki, tagids.id2name(0), NULL);
      }
    }
    morph.tag_print_eos(out);
  };

  /*------------------------------------------------------------
   * public methods: high-level: tagging: output
   */
  inline void print_token_analyses(FILE *out,
				   const mootTokString &token, 
				   const mootTagString &tagstr,
				   const TagMorphAnalysisSet *tms)
  {
    fputs(token.c_str(), out);
    set<TagMorphAnalysis>::const_iterator tmsi;

    if (morph.want_mabbaw_format) {
      /*-- ambiguous, strings, all features, mabbaw-style */
      fprintf(out, ": %d Analyse(n)\n", tms ? tms->analyses.size() : 0);
      if (!want_pos_only) {
	//-- Print all analyses
	if (tms) {
	  for (tmsi = tms->analyses.begin(); tmsi != tms->analyses.end(); tmsi++) {
	    fputc('\t', out);
	    //-- Print the tag first
	    fputc('\t', out);
	    fputs(tagstr.c_str(), out);
	    fputc(':', out); 
	    fputs(tmsi->c_str(), out);
	    fputc('\n', out);
	  }
	}
      }
      fputc('\n', out);
    } else {
      /*-- ambiguous, strings, all features, one tok/line */
      //-- Print the tag first
      fputc('\t', out);
      fputs(tagstr.c_str(), out);
      if (!want_pos_only) {
	//-- Print all analyses
	if (tms) {
	  for (tmsi = tms->analyses.begin(); tmsi != tms->analyses.end(); tmsi++) {
	    fputc('\t', out);
	    fputs(tmsi->c_str(), out);
	  }
	}
      }
      fputc('\n', out);
    }
  };
  //@}


  /// \name Low-level Viterbi methods
  //@{
  /**
   * Low-level: Step a single Viterbi iteration: 'tmm' is NOT added to 'tmmaps' (that
   * should happen at a higher level).
   */
  inline void _viterbi_step_sub(TokID tokid,
				ClassID classid,
				TagMorphMap *tmm,
				ViterbiColumn *col=NULL)
  {
    ViterbiNode   *nod;
    if (!col) {
      //-- Get next column
      col = viterbi_get_column();
      col->col_prev = vtable;
      col->nodes    = NULL;
    }

    //-- Get Token & Class probability table(s)
    const LexProbSubTable      &lps  = lexprobs[tokid];
    const LexClassProbSubTable &lcps = lcprobs[classid];

    //-- for each possible destination tag 'vtagid'
    LexProbSubTable::const_iterator      lpsi;
    LexClassProbSubTable::const_iterator lcpsi;
    for (TagMorphMap::iterator tmmi = tmm->begin(); tmmi != tmm->end(); tmmi++) {
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
      nod->mas      = &(tmmi->second);
      col->nodes    = nod;
    }

    if (col->nodes == NULL) {
      //-- oops: we haven't found anything...
      if (tmm == &(tmm_fallback)) {
	//-- argh: we STILL haven't found anything...
	_viterbi_push(0, tmm, col);                          //-- use "UNKNOWN" tag
      } else {
	_viterbi_step_sub(tokid, classid, &(tmm_fallback));  //-- try fallback morphmap
      }
    } else{
      //-- add new column to state table
      vtable = col;
    }
  };

  /*------------------------------------------------------------
   * Viterbi: when you know the tag
   */
  inline void _viterbi_push(TagID tagid, TagMorphMap *tmm=NULL, ViterbiColumn *col=NULL)
  {
    ViterbiNode   *nod;
    if (!col) {
      //-- Get next column
      col = viterbi_get_column();
      col->col_prev = vtable;
      col->nodes    = NULL;
    }

    //-- for the destination tag 'vtagid'
    vtagid = tagid >= n_tags ? 0 : tagid;

    //-- find best previous tag by n-gram probabilites: store information in vbestpr,vbestpn
    viterbi_find_best_prevnode(vtagid, tagp(vtagid));

    //-- update state table column for current destination tag
    nod           = viterbi_get_node();
    nod->tagid    = vtagid;
    nod->prob     = vbestpr;
    nod->pth_prev = vbestpn;
    nod->row_next = col->nodes;
    nod->mas      = tmm ? &((*tmm)[vtagid]) : NULL;
    col->nodes    = nod;

    //-- add new column to state table
    vtable = col;
  };
  //@}

  /*------------------------------------------------------------
   * public methods: mnemonic wanking
   */

  /// \name Trash-stack methods
  //@{

  /**
   * Returns a pointer to an unused ViterbiNode, possibly allocating a new one.
   * We have to redefine this here since we've changed the size of a state-table
   * node.  Argh.
   */
  inline ViterbiNode *viterbi_get_node(void) {
    ViterbiNode *nod;
    if (trash_nodes != NULL) {
      nod = (ViterbiNode *)trash_nodes;
      trash_nodes = nod->row_next;
    } else {
      nod = new ViterbiNode();
    }
    return nod;
  };


  /**
   * Returns a pointer to an unused TagMorphMap, possibly allocating a new one.
   * Marks returned item as used.
   */
  inline TagMorphMap *get_tmp_morphmap(void) {
    TmpTagMorphMap *ttmm = NULL;
    if (ttmms_free != NULL) {
      ttmm       = ttmms_free;
      ttmms_free = ttmm->next;
    } else {
      ttmm = new TmpTagMorphMap();
    }

    //-- mark as used
    ttmm->next = ttmms_used;
    ttmms_used = ttmm;

    //-- return the guts
    return &(ttmm->tmmap);
  };
  //@}

  /// \name Class-ID lookup utilities
  //@{

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
   * public methods: ClassID lookup : from TagMorphMap
   */

  /**
   * Get the ClassID corresponding to the set of tags whose IDs are keys
   * of 'tmm'. No cache is used.
   */
  inline ClassID morphmap2classid(const TagMorphMap *tmm)
  {
    //-- generate class-name, then generate class if necessary
    return get_morphmap_classid(tmm);
  };

  /**
   * Get the ClassID corresponding to the set of tags whose IDs are keys
   * of 'tmm'.  No cached information is used.
   */
  inline ClassID get_morphmap_classid(const TagMorphMap *tmm)
  {
    return class2classid(get_morphmap_class(tmm));
  };

  /** Convert a TagMorphMap to a LexClass, no caches involved. */
  inline LexClass &get_morphmap_class(const TagMorphMap *tmm)
  {
    lexclass_tmp.clear();
    for (TagMorphMap::const_iterator tmmi = tmm->begin(); tmmi != tmm->end(); tmmi++) {
      lexclass_tmp.insert(tmmi->first);
    }
    return lexclass_tmp;
  };
  //@}

  /*------------------------------------------------------------
   * public methods: TagMorphMap generation / lookup
   */
  /// \name TagMorphMap generation / lookup
  //@{

  /**
   * Looks up (possibly empty) TagID->MorphAnalysisSet map for 'token'.  Uses
   * cached information if available.
   */
  inline TagMorphMap *token2morphmap(const mootTokString &token, TokID tokid=0)
  {
    //-- "unknown" token check : must call morphology
    if (tokid==0) {
      return get_token_morphmap(token,get_tmp_morphmap());
    }
    //-- check the cache
    TokMorphCache::iterator tmci = morphcache.find(tokid);
    if (tmci != morphcache.end()) {
      return &(tmci->second);
    }
    //-- when all else fails, call morphology
    return get_token_morphmap(token,get_tmp_morphmap());
  };

  /**
   * Builds (possibly empty) TagID->MorphAnalysisSet map for 'token' by morphological analysis.
   * No cached information is used.
   */
  inline TagMorphMap *get_token_morphmap(const mootTokString &token, TagMorphMap *tmm=NULL)
  {
    return analyses2morphmap(morph.tag_token(token), tmm);
  };

  /** Convert a raw morphogical analysis-set to a TagMorphMap */
  inline TagMorphMap *analyses2morphmap(const MorphAnalysisSet &ma, TagMorphMap *tmm=NULL)
  {
    if (tmm == NULL) tmm = get_tmp_morphmap();
    tmm->clear();
    for (MorphAnalysisSet::const_iterator mai = ma.begin(); mai != ma.end(); mai++) {
      TagID tagid = vector2tagid(*mai);
      if (tagid == 0) continue; //-- ignore the "unknown" tag
      (*tmm)[tagid].insert(*mai, morph);
    }
    return tmm;
  };
  //@}

  /// \name Vector->TagID conversion
  //@{

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
  //@}


  /*------------------------------------------------------------
   * public methods: low-level: debugging
   */

  /// \name Debugging
  //@{

  /** Debugging method: dump basic CHMM contents to a text file. */
  void txtdump(FILE *file);

  /* Debugging method: dump current Viterbi state-table column to a text file */
  //void viterbi_txtdump(FILE *file);
  //@}
};

moot_END_NAMESPACE

#endif /* _moot_CHMM_H */
