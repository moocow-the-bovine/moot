/* -*- Mode: C++ -*- */
#include "bgttest.h"

/*----------------------------------------------------------------------
 * defines
 *----------------------------------------------------------------------*/
#define BENCH_CLASS bgtRecVV

/*----------------------------------------------------------------------
 * Class definition
 *----------------------------------------------------------------------*/
class BENCH_CLASS {
public:
  /** Type for lookup subtable: TagID->ProbT. */
  typedef vector<ProbT> BgSubtable;

  /** Type for lookup table: TagID->(TagID->ProbT). */
  typedef vector<BgSubtable> BgTable;

public:
  BgTable bgprobs;

private:
  size_t    n_tags;
  //TagSet  thetags;

public:
  //-----------------------------------
  // constructor
  BENCH_CLASS(void) :
    n_tags(0)
  {};

  //-----------------------------------
  // destructor
  ~BENCH_CLASS(void)
  {};

  //-----------------------------------
  // lookup(ptag,tag)
  inline const ProbT lookup(const TagID ptagid, const TagID tagid) const
  {
    if (ptagid >= bgprobs.size()) return 0;
    const BgSubtable &bgs = bgprobs[ptagid];
    if (tagid >= bgs.size()) return 0;
    return bgs[tagid];
  };

  //-----------------------------------
  //-- manipulation
  inline void add(const TagID ptagid, const TagID tagid, const ProbT count)
  {
    //-- add subtable if not present
    if (ptagid >= bgprobs.size()) bgprobs.resize(ptagid+1);

    //-- add count
    BgSubtable &bgs = bgprobs[ptagid];
    if (tagid >= bgs.size()) {
      bgs.resize(tagid+1);
      bgs[tagid] = count;
    } else {
      bgs[tagid] += count;
    }
  };

  //-----------------------------------
  //-- bench
  void bench_lookup(const size_t iters) {
    ProbT prob;
    TagID ptagid, tagid;
    for (size_t i = 0; i < iters; i++) {
      ptagid = rand()%NTAGS;
      tagid  = rand()%NTAGS;
      prob   = lookup(ptagid,tagid);
    }
  };
};

/*----------------------------------------------------------------------
 * MAIN
 *----------------------------------------------------------------------*/

int main(int argc, char **argv) {
  //-- vars
  TagID ptagid, ntags, tagu;

  //-- progname
  progname = *argv;

  //-- init
  BENCH_CLASS bg;
  gettimeofday(&t1, NULL);
  for (ptagid = 0; ptagid < NTAGS; ptagid++) {
    ntags = MIN_NPTAGS + (rand()%(MAX_NPTAGS-MIN_NPTAGS));
    for (tagu = 0; tagu < ntags; tagu++) {
      bg.add(ptagid, rand()%NTAGS, rand()%128);
    }
  }
  gettimeofday(&t2, NULL);
  benchinfo(INIT_NAME, 1, t1, t2);

  //-- lookup(ptag,tag)
  gettimeofday(&t1, NULL);
  bg.bench_lookup(BENCH_ITERS);
  gettimeofday(&t2, NULL);
  benchinfo(BENCH_NAME, BENCH_ITERS, t1, t2);

  //-- memcheck
  fprintf(stderr, "\n%s: Done. Memory usage? ", progname);
  char memusg[512];
  scanf("%s", memusg);
  printf("\n%s: Memory Usage = %s\n\n", progname, memusg);

  return 0;
}
