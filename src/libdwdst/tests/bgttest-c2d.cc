/* -*- Mode: C++ -*- */
#include "bgttest.h"
#include "string.h"

/*----------------------------------------------------------------------
 * defines
 *----------------------------------------------------------------------*/
#define BENCH_CLASS bgtC2D

/*----------------------------------------------------------------------
 * Class definition
 *----------------------------------------------------------------------*/
class BENCH_CLASS {
public:
  ProbT *bgprobs;  /**< Probability table: p(tag|ptag) = bgprobs[(ptag*n_tags)+tag]; */

private:
  size_t    n_tags;
  //TagSet  thetags;

public:
  //-----------------------------------
  // constructor
  BENCH_CLASS(void) :
    bgprobs(NULL),
    n_tags(0)
  {};

  //-----------------------------------
  // destructor
  ~BENCH_CLASS(void)
  {};

  //-----------------------------------
  // initialization
  void initialize(size_t ntags=0)
  {
    n_tags = ntags;
    if (bgprobs) free(bgprobs);
    bgprobs = (ProbT *)malloc(ntags*ntags*sizeof(ProbT));
    if (!bgprobs) {
      fprintf(stderr, "Error: could not allocate bigram table!");
      exit(1);
    }
    memset(bgprobs, 0, ntags*ntags*sizeof(ProbT));
  };

  //-----------------------------------
  // lookup(ptag,tag)
  inline const ProbT lookup(const TagID ptagid, const TagID tagid) const
  {
    if (ptagid >= n_tags || tagid >= n_tags || !bgprobs) return 0;
    return bgprobs[(ptagid*n_tags)+tagid];
  };

  //-----------------------------------
  //-- manipulation
  inline void add(const TagID ptagid, const TagID tagid, const ProbT count)
  {
    //-- add subtable if not present
    if (ptagid >= n_tags || tagid >= n_tags || !bgprobs) {
      if (ptagid >= n_tags) {
	fprintf(stderr, "add(): Error: ptagid=%u >= n_tags=%u\n", ptagid, n_tags);
      }
      if (tagid >= n_tags) {
	fprintf(stderr, "add(): Error: tagid=%u >= n_tags=%u\n", tagid, n_tags);
      }
      if (!bgprobs) {
	fprintf(stderr, "add(): Error: bgprobs==NULL\n");
      }
      exit(2);
    }

    //-- add count
    bgprobs[(ptagid*n_tags)+tagid] += count;
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
  bg.initialize(NTAGS);
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
