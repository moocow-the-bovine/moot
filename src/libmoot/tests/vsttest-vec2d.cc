/* -*- Mode: C++ -*- */
#include "vsttest.h"
#include <string.h>

/*----------------------------------------------------------------------
 * defines
 *----------------------------------------------------------------------*/
#define BENCH_CLASS vstVec2D

/*----------------------------------------------------------------------
 * Class definition
 *----------------------------------------------------------------------*/
class BENCH_CLASS {
public:
  /** Type for a state-table entry (mapped from tag-ids) */
  class ViterbiNode {
  public:
    TagID             tagid; /**< tag-ID for this node */
    ProbT              prob; /**< Probability of best path to this node */
    ViterbiNode   *pth_prev; /**< Previous tag-id */
  public:
    ViterbiNode(const TagID node_tagid=0, const ProbT node_prob=0.0, ViterbiNode *node_prev=NULL)
      : tagid(node_tagid),
	prob(node_prob),
	pth_prev(node_prev)
    {};

    ~ViterbiNode(void) {};
  };

  /** Type for a Viterbi state-table column (C-array of nodes) */
  typedef ViterbiNode *ViterbiColumn;

  /** Type for a whole Viterbi state-table */
  typedef list<ViterbiColumn> ViterbiTable;

  /**
   * Type for possible previous-tag-sets for (modified) Viterbi
   */
  //typedef vector<TagID> ViterbiTagSet;
  typedef vstInSent::TagSet ViterbiTagSet;

public:
  TagID             start_tagid;
  size_t            n_tags;

  //------ low-level data: for Viterbi algorithm
  ViterbiTable      vtable;   /**< State table for Viterbi algorithm */
  ViterbiColumn     boscol;   /**< Constant beginning-of-sentence column */

  //ViterbiTagSet     vtags;    /**< Valid current tags */
  ViterbiTagSet     pvtags;   /**< Valid previous tags */

private:

  //------ private data: trash
  slist<ViterbiColumn> trash_columns;

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
    n_tags(0),
    boscol(NULL)
  {};

  //-----------------------------------
  //-- destructor
  ~BENCH_CLASS(void)
  {
    cleanup();
    if (boscol) free(boscol);
    boscol = NULL;
  };

  //-----------------------------------
  // initialization
  inline void initialize(size_t ntags=0, size_t sentlen=0)
  {
    if (ntags != n_tags) {
      n_tags = ntags;

      //-- de-allocate anything we've already got
      cleanup();

      //-- allocate initial Viterbi column
      if (boscol) free(boscol);
      boscol = get_column();
    }

    //-- setup initial Viterbi column
    ViterbiNode *bosnod = boscol + start_tagid;
    bosnod->tagid = start_tagid;
    bosnod->prob  = 1.0;
    bosnod->pth_prev = NULL;

    //-- setup tag-tracker(s)
    //vtags.reserve(ntags);
    pvtags.reserve(ntags);

    //-- setup 'eostags' constant
    eostags.clear();
    eostags.push_back(start_tagid);

    //-- pre-allocate for expected sentence length
    for (size_t i = 0; i < sentlen; i++) {
      trash_columns.push_front(get_column());
    }
  };

  //-----------------------------------
  // column allocation
  inline ViterbiColumn get_column(void) {
    ViterbiColumn col;
    if (trash_columns.begin() != trash_columns.end()) {
      col = trash_columns.front();
      trash_columns.pop_front();
    } else {
      col = (ViterbiColumn)malloc(n_tags*sizeof(ViterbiNode));
    }
    return col;
  };

  //-----------------------------------
  // clear (state reset)
  inline void clear(void)
  {
    //-- move to trash
    for (ViterbiTable::const_iterator vti = vtable.begin();
	 vti != vtable.end();
	 vti = vtable.begin())
      {
	if (*vti != boscol) trash_columns.push_front(*vti);
	vtable.pop_front();
      }

    //-- add BOS entry
    vtable.push_front(boscol);

    //-- set valid previous tags
    //vtags.clear();
    //vtags.push_back(start_tagid);
    pvtags.clear();
    pvtags.push_back(start_tagid);
  };

  //-----------------------------------
  // cleanup (de-allocation)
  void cleanup(void)
  {
    clear();
    vtable.clear();
    free(boscol);

    //-- reset to NULL
    boscol = NULL;
  };


  //-----------------------------------
  //-- step
  inline void step(const TokID tokid, const vstInSent::TagSet &tagset)
  {
    ViterbiColumn pcol = vtable.back();
    ViterbiColumn  col = get_column();
    vtable.push_back(col);

    //-- swap previous & current tag-lists
    //vtags.swap(pvtags);
    //vtags.clear();

    //-- for each destination tag 'vtagid'
    for (vstInSent::TagSet::const_iterator tsi = tagset.begin(); tsi != tagset.end(); tsi++)
      {
	vtagid = *tsi;

	//-- begin find_best_prevtag()
	vbestpr = -1.0;
	for (ViterbiTagSet::const_iterator pvtagi = pvtags.begin();
	     pvtagi != pvtags.end();
	     pvtagi++)
	  {
	    ViterbiNode *pnod = pcol + *pvtagi;
	    vtagpr = (1.0/pnod->prob) * rand();
	    if (vtagpr > vbestpr)  {
	      vbestpr = vtagpr;
	      vbestpn = pnod;
	    }
	  }
	//-- end find_best_prevtag()

	//-- update state table column for current destination tag
	ViterbiNode *vnode = col + vtagid;
	vnode->tagid = vtagid;
	vnode->prob  = vbestpr;
	vnode->pth_prev = vbestpn;

	//-- remember possible destination tags
	//vtags.push_back(vtagid);
      }

    //-- swap previous & current tag-lists
    pvtags = tagset;

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
    for (nod = (vtable.back()+start_tagid)->pth_prev; nod != NULL; nod = nod->pth_prev) {
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
