/* -*- Mode: C++ -*- */
#include "vsttest.h"

/*----------------------------------------------------------------------
 * defines
 *----------------------------------------------------------------------*/
#define BENCH_CLASS vstPtrs

/*----------------------------------------------------------------------
 * Class definition
 *----------------------------------------------------------------------*/
class BENCH_CLASS {
public:
  /** Type for a state-table entry (linked-list columns) */
  struct ViterbiNode {
    TagID tagid;                   /**< Tag-ID for this node */
    ProbT prob;                    /**< Probability of best path to this node */
    struct ViterbiNode *pth_prev;  /**< Previous node in best path */
    struct ViterbiNode *row_next;  /**< Next node in column */
  };

  /** Type for a state-table column (the state-table itself) */
  struct ViterbiColumn {
    //TokID          tokid;    /**< Token-ID for this column */
    ViterbiNode   *nodes;    /**< Column nodes */
    ViterbiColumn *col_prev; /**< Previous column */
  };

public:
  TagID             start_tagid;
  //TokID             start_tokid;

  //------ low-level data: for Viterbi algorithm
  ViterbiColumn *vtable;   /**< State table for Viterbi algorithm */

private:
  //------ private data: trash bins
  ViterbiNode   *trash_nodes;
  ViterbiColumn *trash_columns;

  //------ private data: counters
  ProbT             vbestpr;    /**< Best probability for viterbi_step() */
  //TagID             vbestpt;    /**< Best previous tag for viterbi_best() */
  ViterbiNode      *vbestpn;    /**< Best previous node for viterbi_step() */
  TagID             vtagid;     /**< Current tag-id under consideration for viterbi_step() */
  ProbT             vtagpr;     /**< Probability for current tag-id for viterbi_step() */

  //------ eos-tagset
  vstInSent::TagSet eostags;    /**< constant */

public:
  //-----------------------------------
  //-- constructor
  BENCH_CLASS(void) :
    start_tagid(0),
    //start_tokid(0),
    vtable(NULL),
    trash_nodes(NULL),
    trash_columns(NULL)
  {};

  //-----------------------------------
  //-- destructor
  ~BENCH_CLASS(void)
  {
    cleanup();
  };

  //-----------------------------------
  // list utilities: nodes
  inline ViterbiNode *get_node(void) {
    ViterbiNode *nod;
    if (trash_nodes != NULL) {
      nod = trash_nodes;
      trash_nodes = nod->row_next;
    } else {
      nod = new ViterbiNode();
    }
    return nod;
  };

  //-----------------------------------
  //-- list utilities: columns
  inline ViterbiColumn *get_column(void) {
    ViterbiColumn *col;
    if (trash_columns != NULL) {
      col = trash_columns;
      trash_columns = col->col_prev;
    } else {
      col = new ViterbiColumn();
    }
    return col;
  };

  //-----------------------------------
  // clear (state reset)
  inline void clear(bool forDestroy=false)
  {
    //-- move to trash
    ViterbiColumn *col, *col_next;
    ViterbiNode   *nod, *nod_next;
    for (col = vtable; col != NULL; col = col_next) {
      col_next      = col->col_prev;
      col->col_prev = trash_columns;
      trash_columns = col;
      for (nod = col->nodes; nod != NULL; nod = nod_next) {
	nod_next      = nod->row_next;
	nod->row_next = trash_nodes;
	trash_nodes   = nod;
      }
    }
    //-- add BOS entry
    vtable = get_column();

    //vtable->tokid = start_tokid;
    vtable->col_prev = NULL;
    nod = vtable->nodes = get_node();
    nod->tagid = start_tagid;
    nod->prob  = 1.0;
    nod->row_next = NULL;
    nod->pth_prev = NULL;
  };

  //-----------------------------------
  // cleanup (de-allocation)
  void cleanup(void)
  {
    ViterbiColumn *col, *col_next;
    ViterbiNode   *nod, *nod_next;

    //-- free columns and nodes
    for (col = vtable; col != NULL; col = col_next) {
      col_next      = col->col_prev;
      for (nod = col->nodes; nod != NULL; nod = nod_next) {
	nod_next    = nod->row_next;
	delete nod;
      }
      delete col;
    }

    //-- free trashed cols
    for (col = trash_columns; col != NULL; col = col_next) {
      col_next = col->col_prev;
      delete col;
    }

    //-- free trashed nodes
    for (nod = trash_nodes; nod != NULL; nod = nod_next) {
      nod_next = nod->row_next;
      delete nod;
    }
    
    //-- reset to NULL
    vtable        = NULL;
    trash_nodes   = NULL;
    trash_columns = NULL;
  };


  //-----------------------------------
  // initialization
  inline void initialize(size_t ntags=0, size_t sentlen=0)
  {
    clear();
    //-- setup 'eostags' constant
    eostags.clear();
    eostags.push_back(start_tagid);
  };

  //-----------------------------------
  //-- step
  inline void step(const TokID tokid, const vstInSent::TagSet &tagset)
  {
    ViterbiColumn *col = get_column();
    ViterbiNode   *pnod, *nod;
    col->col_prev = vtable;
    col->nodes = NULL;
    //col->tokid = tokid;

    //-- for each destination tag 'vtagid'
    for (vstInSent::TagSet::const_iterator tsi = tagset.begin(); tsi != tagset.end(); tsi++)
      {
	vtagid = *tsi;

	//-- begin find_best_prevtag()
	vbestpr = -1.0;
	for (pnod = vtable->nodes; pnod != NULL; pnod = pnod->row_next) {
	  vtagpr = (1.0/pnod->prob) * rand();
	  if (vtagpr > vbestpr)  {
	    vbestpr = vtagpr;
	    vbestpn = pnod;
	  }
	}
	//-- end find_best_prevtag()

	//-- update state table column for current destination tag
	nod = get_node();
	nod->tagid = vtagid;
	nod->prob  = vbestpr;
	nod->pth_prev = pnod;
	nod->row_next = col->nodes;
	col->nodes = nod;
      }

    //-- add column to state table
    vtable = col;

    return;
  };

  //-- finish
  inline void finish(void) { step(0, eostags); };


  //-- (clear)+(n*step)+(bestpath_iter)
  void bench(const vstInSent &sent)
  {
    //-- clear();
    clear();

    //-- n*step()
    vstInSent::TokVec::const_iterator    tvi;
    vstInSent::TagSetVec::const_iterator tsi;
    for (tvi =  sent.tokids.begin()  , tsi  = sent.tagsets.begin();
	 tvi != sent.tokids.end()   && tsi != sent.tagsets.end();
	 tvi ++                      , tsi++)
      {
	step(*tvi, *tsi);
      }

    //-- bestpath_iter(n)
    finish();
    ViterbiNode *nod;
    TagID tagid;
    for (nod = vtable->nodes; nod != NULL; nod = nod->pth_prev) {
      tagid = nod->tagid; //-- dummy
    }

    return;
  };
};


/*----------------------------------------------------------------------
 * MAIN
 *----------------------------------------------------------------------*/
int main (int argc, char **argv) {
  //-- progname
  progname = *argv;

  //-- init
  BENCH_CLASS vt;
  gettimeofday(&t1, NULL);
  vt.initialize(NTAGS, SLEN_INIT);
  gettimeofday(&t2, NULL);
  benchinfo(INIT_NAME, 1, t1, t2);

  //-- prep
  vstInSent sent(SLEN_LONG_MAX+1);

  //-- bench: (clear)+(n*step)+(bestpath_iter)
  size_t slen, ntoks = 0;
  gettimeofday(&t1, NULL);
  for (size_t i = 0; i < BENCH_ITERS; i++) {
    slen = SLEN_MIN+(rand()%(SLEN_MAX-SLEN_MIN));
    sent.generate(slen);
    vt.bench(sent);
    ntoks += slen;
  }
  gettimeofday(&t2, NULL);
  benchinfo(BENCH_NAME, ntoks * BENCH_ITERS, t1, t2);

  //-- bench outliers: (clear)+(n*step)+(bestpath_iter)
  gettimeofday(&t1, NULL);
  ntoks = 0;
  for (size_t i = 0; i < BENCH_LONG_ITERS; i++) {
    slen = SLEN_LONG_MIN+(rand()%(SLEN_LONG_MAX-SLEN_LONG_MIN));
    sent.generate(slen);
    vt.bench(sent);
    ntoks += slen;
  }
  gettimeofday(&t2, NULL);
  benchinfo(BENCH_LONG_NAME, ntoks * BENCH_LONG_ITERS, t1, t2);

  //-- check memory usage
  fprintf(stderr, "\n%s: Done. Memory usage? ", progname);
  char memusg[512];
  scanf("%s", memusg);
  printf("\n%s: Memory Usage = %s\n\n", progname, memusg);

  return 0;
}
