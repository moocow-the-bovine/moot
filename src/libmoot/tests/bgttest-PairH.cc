/* -*- Mode: C++ -*- */
#include "bgttest.h"

/*----------------------------------------------------------------------
 * defines
 *----------------------------------------------------------------------*/
#define BENCH_CLASS bgtPairH

/*----------------------------------------------------------------------
 * Class definition
 *----------------------------------------------------------------------*/
class BENCH_CLASS {
public:
  /** Type for lookup key */
  typedef pair<TagID,TagID> IDPair;

  /** Hash-function struct for IDPair, needed by hash_map */
  struct IDPairHashFcn {
  public:
    inline size_t operator()(const IDPair &x) const
    {
      return
	//(x.first<<5)-x.first + x.second;
	5*x.first + x.second;
    };
  };

  /** Equality struct for IDPair, needed by hash_map */
  struct IDPairEqualFcn {
  public:
    inline size_t operator()(const IDPair &x, const IDPair &y) const
    {
      return x.first == y.first && x.second == y.second;
    };
  };

  /** Type for (pair(id,id)->probability) lookup table. */
  typedef hash_map<IDPair,ProbT,IDPairHashFcn,IDPairEqualFcn> IDPairProbTable;

  /** Type for lookup table: pair(ptagid,tagid)->p(tagid|ptagid) */
  typedef IDPairProbTable BgTable;

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
    BgTable::const_iterator bgi = bgprobs.find(IDPair(ptagid,tagid));
    if (bgi == bgprobs.end()) return 0;
    return bgi->second;
  };

  //-----------------------------------
  //-- manipulation
  inline void add(const TagID ptagid, const TagID tagid, const ProbT count)
  {
    BgTable::iterator bgi = bgprobs.find(IDPair(ptagid,tagid));
    if (bgi == bgprobs.end()) {
      bgprobs[IDPair(ptagid,tagid)] = count;
    } else {
      bgi->second += count;
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
