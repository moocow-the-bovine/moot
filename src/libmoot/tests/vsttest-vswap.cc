/* -*- Mode: C++ -*- */
#include "vsttest.h"

/*----------------------------------------------------------------------
 * defines
 *----------------------------------------------------------------------*/
#define BENCH_CLASS vstVswap

/*----------------------------------------------------------------------
 * Class definition
 *----------------------------------------------------------------------*/
class BENCH_CLASS {
public:
  /** Type for an unweighted state-sequence (read: "tag-sequence") */
  typedef vector<TagID>   StateSeq;

  /** Type for a weighted state-sequence: used for entries in the Viterbi state-table. */
  struct WeightedStateSeq {
  public:
    ProbT    prob;
    StateSeq path;
  };

  /** Type for the Viterbi state-table. Keys: TagID (not anymore!)
   * Values: WeightedStateSeq : best pair (prob(path),path)
   *         such that 'path' ends in TagID.
   */
  typedef vector<WeightedStateSeq> ViterbiStateTable;

  /**
   * Type for possible previous-tag-sets for (modified) Viterbi
   */
  //typedef vector<TagID> ViterbiTagSet;
  typedef vstInSent::TagSet ViterbiTagSet;

public:
  TagID             start_tagid;

  //------ low-level data: for Viterbi algorithm
  ViterbiStateTable vtable;     /**< Low-level state table for Viterbi algorithm */
  ViterbiStateTable pvtable;    /**< Low-level state table for Viterbi algorithm (previous iter) */

  ViterbiTagSet     vtags;      /**< Low-level tagset for Viterbi algorithm */
  ViterbiTagSet     pvtags;     /**< Low-level tagset for Viterbi algorithm (previous iter) */

private:
  //------ private data
  ProbT             vbestpr;    /**< Best probability for viterbi_step() */
  TagID             vbestpt;    /**< Best previous tag for viterbi_best() */
  TagID             vtagid;     /**< Current tag-id under consideration for viterbi_step() */
  ProbT             vtagpr;     /**< Probability for current tag-id for viterbi_step() */

  //------ eos-sequence
  ViterbiTagSet     eostags;    /**< constant */

public:
  //-- constructor, destructor
  BENCH_CLASS(void) : start_tagid(0) {};
  ~BENCH_CLASS(void)
  {
    vtable.clear();
    pvtable.clear();
    vtags.clear();
    pvtags.clear();
  };

  //-- initialization
  inline void initialize(size_t ntags=0, size_t sentlen=0)
  {
    vtable.resize(ntags);
    pvtable.resize(ntags);
    vtags.resize(ntags);
    pvtags.resize(ntags);
    clear();
    //-- setup eos-tags
    eostags.clear();
    eostags.push_back(start_tagid);
  };

  //-- clear
  inline void clear(void) {
    if (vtable.size() == 0) return;
    //-- manipulate the special 'bos' entry
    WeightedStateSeq &startseq = vtable[start_tagid];
    startseq.path.clear();
    startseq.path.push_back(start_tagid);
    startseq.prob = 1.0;
    //-- set valid previous tags
    vtags.clear();
    vtags.push_back(start_tagid);
  };

  //-- step
  inline void step(const TokID tokid, const vstInSent::TagSet &tagset)
  {
    vtable.swap(pvtable);
    vtags.swap(pvtags);
    vtags.clear();
    //vtable.clear();

    //-- for each destination tag
    for (vstInSent::TagSet::const_iterator tsi = tagset.begin(); tsi != tagset.end(); tsi++)
      {
	vtagid = *tsi;

	//-- begin find_best_prevtag()
	vbestpr = -1.0;
	for (ViterbiTagSet::const_iterator pvtagi = pvtags.begin();
	     pvtagi != pvtags.end();
	     pvtagi++)
	  {
	    const WeightedStateSeq &pwsseq = pvtable[*pvtagi];
	    vtagpr = (1.0/pwsseq.prob) * rand();
	    if (vtagpr > vbestpr)  {
	      vbestpr = vtagpr;
	      vbestpt = *pvtagi;
	    }
	  }
	//-- end find_best_prevtag()

	//-- update state table for current destination tag
	WeightedStateSeq &wsseq = vtable[vtagid];
	wsseq.prob = vbestpr;
	wsseq.path = pvtable[vbestpt].path;
	wsseq.path.push_back(vtagid);

	//-- remember possible destination tags
	vtags.push_back(vtagid);
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
    const WeightedStateSeq &bestseq = vtable[start_tagid];
    TagID tagid;
    for (StateSeq::const_iterator bpi = bestseq.path.begin(); bpi != bestseq.path.end(); bpi++)
      {
	tagid = *bpi; //-- dummy
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
