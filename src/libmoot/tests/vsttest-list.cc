/* -*- Mode: C++ -*- */
#include "vsttest.h"

/*----------------------------------------------------------------------
 * defines
 *----------------------------------------------------------------------*/
#define BENCH_CLASS vstMap

/*----------------------------------------------------------------------
 * Class definition
 *----------------------------------------------------------------------*/
class BENCH_CLASS {
public:
  /** Type for a state-table entry (mapped from tag-ids) */
  class ViterbiNode {
  public:
    TagID            tagid; /**< tag-ID for this node */
    ProbT            prob;  /**< Probability of best path to this node */
    ViterbiNode *pth_prev;  /**< Previous node in best path */
  public:
    ViterbiNode(const TagID node_tagid=0, const ProbT node_prob=0.0, ViterbiNode *node_prev=NULL)
      : tagid(node_tagid),
	prob(node_prob),
	pth_prev(node_prev)
    {};

    ~ViterbiNode(void) {};
  };

  /** Type for a Viterbi state-table column */
  typedef list<ViterbiNode> ViterbiColumn;

  /** Type for a whole Viterbi state-table */
  typedef list<ViterbiColumn> ViterbiTable;

public:
  TagID             start_tagid;

  //------ low-level data: for Viterbi algorithm
  ViterbiTable      vtable;   /**< State table for Viterbi algorithm */
  ViterbiColumn     boscol;   /**< Constant beginning-of-sentence column */

private:

  //------ private data: counters
  ProbT             vbestpr;    /**< Best probability for viterbi_step() */
  //ViterbiColumn::const_iterator vbestpi;  /**< Best previous tag-iterator for viterbi_step() */
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
    start_tagid(0)
  {};

  //-----------------------------------
  //-- destructor
  ~BENCH_CLASS(void)
  {
    cleanup();
  };

  //-----------------------------------
  // initialization
  inline void initialize(size_t ntags=0, size_t sentlen=0)
  {
    //-- set up initial Viterbi column
    boscol.clear();
    boscol.push_back(ViterbiNode(start_tagid, 1.0, NULL));
    //-- setup 'eostags' constant
    eostags.clear();
    eostags.push_back(start_tagid);
  };


  //-----------------------------------
  // clear (state reset)
  inline void clear(void)
  {
    //-- clear state table
    vtable.resize(1);

    //-- add BOS entry
    vtable.front() = boscol;
  };

  //-----------------------------------
  // cleanup (de-allocation)
  void cleanup(void)
  {
    vtable.clear();
  };


  //-----------------------------------
  //-- step
  inline void step(const TokID tokid, const vstInSent::TagSet &tagset)
  {
    ViterbiColumn &pcol = vtable.back();
    vtable.push_back(ViterbiColumn());
    ViterbiColumn &col = vtable.back();

    //-- for each destination tag 'vtagid'
    for (vstInSent::TagSet::const_iterator tsi = tagset.begin(); tsi != tagset.end(); tsi++)
      {
	vtagid = *tsi;

	//-- begin find_best_prevtag()
	vbestpr = -1.0;
	for (ViterbiColumn::iterator pci = pcol.begin(); pci != pcol.end(); pci++)
	  {
	    vtagpr = (1.0/pci->prob) * rand();
	    if (vtagpr > vbestpr)  {
	      vbestpr = vtagpr;
	      vbestpn = &(*pci);
	    }
	  }
	//-- end find_best_prevtag()

	//-- update state table column for current destination tag
	col.push_back(ViterbiNode(vtagid,vbestpr,vbestpn));
      }
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
    for (nod = vtable.back().back().pth_prev; nod != NULL; nod = nod->pth_prev) {
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
