#include <stdio.h>

#include <vector>

#include <hash_map>
#include <map>

#include <set>
#include <hash_set>


#include <time.h>
#include <sys/time.h>

using namespace std;

/*----------------------------------------------------------------------
 * globals
 *----------------------------------------------------------------------*/
typedef float  ProbT;
typedef size_t TokID;
typedef size_t TagID;

//typedef set<TagID> TagSet;
typedef vector<TagID> TagSet;

char *progname;

/*----------------------------------------------------------------------
 * abstract class (just a spec, really)
 *----------------------------------------------------------------------*/
class LexProbs
{
public:
  LexProbs(void) {};
  ~LexProbs(void) {};
  //-- lookup
  inline const ProbT  lookup(TokID tokid, TagID tagid) const;
  inline const TagSet &tags(TokID tokid);
  //-- manipulation
  void add(TokID tokid, TagID tagid, ProbT count);
};

/*----------------------------------------------------------------------
 * Recursive vector-hash : vector<hash_map<TagID,ProbT> >
 *----------------------------------------------------------------------*/
class LexProbsRecVH : public LexProbs
{
public:
  /** Type for lookup subtable: TagID->ProbT. */
  typedef hash_map<TagID,ProbT> LexProbSubTable;

  /** Type for lookup table: TokID->(TagID->ProbT). */
  typedef vector<LexProbSubTable> LexProbTable;

public:
  LexProbTable lexprobs;

private:
  TagSet       thetags;

public:
  LexProbsRecVH(void) {};
  ~LexProbsRecVH(void) {};

  //-- lookup(tok,tag)
  inline const ProbT lookup(const TokID tokid, const TagID tagid) const
  {
    if (tokid >= lexprobs.size()) return 0;
    const LexProbSubTable &lps = lexprobs[tokid];
    LexProbSubTable::const_iterator lpsi = lps.find(tagid);
    if (lpsi == lps.end()) return 0;
    return lpsi->second;
  };

  //-- lookup(tok)
  inline const TagSet &tags(const TokID tokid)
  {
    thetags.clear();
    if (tokid < lexprobs.size()) {
      const LexProbSubTable &lps = lexprobs[tokid];
      for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++) {
	thetags.push_back(lpsi->first);
      }
    }
    return thetags;
  };

  //-- manipulation
  inline void add(const TokID tokid, const TagID tagid, const ProbT count)
  {
    //-- add subtable if not present
    if (tokid >= lexprobs.size()) lexprobs.resize(tokid+1);

    //-- add count
    LexProbSubTable &lps = lexprobs[tokid];
    LexProbSubTable::iterator lpsi = lps.find(tagid);
    if (lpsi == lps.end()) {
      lps[tagid] = 0;
    } else {
      lpsi->second += count;
    }
  };

  //-- bench
  void bench_lookup(const TokID tokid, const TagSet &tagids) {
    ProbT prob;
    for (TagSet::const_iterator tsi = tagids.begin(); tsi != tagids.end(); tsi++)
      {
	prob = lookup(tokid,*tsi);
      }
  };

  void bench_tags(const TokID tokid) {
    if (tokid >= lexprobs.size()) return;

    ProbT prob;
    LexProbSubTable &lps = lexprobs[tokid];
    for (LexProbSubTable::const_iterator lpsi = lps.begin(); lpsi != lps.end(); lpsi++) {
      prob = lpsi->second;
    }
  };
};



/*----------------------------------------------------------------------
 * Benchmarking
 *----------------------------------------------------------------------*/

void benchinfo(char *name, unsigned iters, timeval started, timeval stopped)
{
  double elapsed = (stopped.tv_sec
		    - started.tv_sec
		    + (double)(stopped.tv_usec - started.tv_usec)
		    / 1000000.0);
  printf("%s: Benchmarked %u iterations of '%s':\n", progname, iters, name);
  printf("%s:   Elapsed : %g sec\n", progname, elapsed);
  printf("%s:     Rate  : %g iters/sec\n", progname, (double)iters / elapsed);
}

/*----------------------------------------------------------------------
 * Bench Defines
 *----------------------------------------------------------------------*/

#define BENCH_CLASS LexProbsRecVH
//#define BENCH_NAME  "LexProbsRecVH"
#define BENCH_ITERS 65536

#define INIT_NAME "(initialize)"
#define GETTAGS_NAME "bench_lookup(tokid)"
#define LOOKUP_NAME "bench_tags(tokid)"

#define TOK_MINTAGS 1
#define TOK_MAXTAGS 10

#define NTOKENS 51277
#define NTAGS   56

#define DOBENCH(iters,code) \
  for (unsigned u = 0; u < iters; u++) { code ; }

/*----------------------------------------------------------------------
 * MAIN
 *----------------------------------------------------------------------*/

int main(int argc, char **argv) {
  timeval  t1,t2;   //-- for timing info
  //ProbT    prob;    //-- for lookup
  unsigned tok, ntags, tagu, u;

  //-- progname
  progname = *argv;

  //-- init
  BENCH_CLASS lp;
  gettimeofday(&t1, NULL);
  for (tok = 0; tok < NTOKENS; tok++) {
    ntags = TOK_MINTAGS + (rand()%(TOK_MAXTAGS-TOK_MINTAGS));
    for (tagu = 0; tagu < ntags; tagu++) {
      lp.add(tok, rand()%NTAGS, rand()%128);
    }
  }
  gettimeofday(&t2, NULL);
  benchinfo(INIT_NAME, 1, t1, t2);

  //-- prep
  vector<TokID>  tokids;
  vector<TagSet> tagsets;
  for (u = 0; u < BENCH_ITERS; u++) {
    tok = rand()%NTOKENS;
    tokids.push_back(tok);
    tagsets.push_back(lp.tags(tok));
  }

  //-- lookup(tok,tag)
  gettimeofday(&t1, NULL);
  for (u = 0; u < BENCH_ITERS; u++) {
    lp.bench_lookup(tokids[u], tagsets[u]);
  }
  gettimeofday(&t2, NULL);
  benchinfo(LOOKUP_NAME, BENCH_ITERS, t1, t2);

  //-- foreach tag \in tags(tok) { lookup(tok,tag); }
  gettimeofday(&t1, NULL);
  for (u = 0; u < BENCH_ITERS; u++) {
    lp.bench_tags(tokids[u]);
  }
  gettimeofday(&t2, NULL);
  benchinfo(GETTAGS_NAME, BENCH_ITERS, t1, t2);

  fprintf(stderr, "\n%s: Done. Memory usage? ", progname);
  char memusg[512];
  scanf("%s", memusg);
  printf("\n%s: Memory Usage = %s\n\n", progname, memusg);

  return 0;
}
