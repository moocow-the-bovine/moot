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
  inline const ProbT  lookup(const TokID tokid, const TagID tagid) const;
  inline const TagSet &tags(const TokID tokid);
  //-- manipulation
  void add(const TokID tokid, const TagID tagid, const ProbT count);

  //-- bench
  void bench_lookup(const TokID tokid, const TagSet &tagids);
  void bench_tags(const TokID tokid);
};


/*----------------------------------------------------------------------
 * separate storage : hash_map
 *----------------------------------------------------------------------*/
class LexProbsSepHV : public LexProbs
{
public:
  typedef pair<size_t,size_t> IDPair;

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

  /**
   * Type for lexical frequency lookup table:
   * (IDPair(TokID,TagID)->p(TokID|TagID))
   */
  typedef IDPairProbTable LexProbTable;

  /** Type for mapping tokids to sets of valid tags */
  //typedef hash_map<TokID,set<TagID> > LexTagMap;
  typedef vector<TagSet> LexTagMap;

public:
  LexProbTable lexprobs;
  LexTagMap    lextags;
  TagSet       emptyset;

public:
  LexProbsSepHV(void) {};
  ~LexProbsSepHV(void) {};

  //-- lookup
  inline const ProbT lookup(const TokID tokid, const TagID tagid) const
  {
    LexProbTable::const_iterator lpi = lexprobs.find(IDPair(tokid,tagid));
    if (lpi == lexprobs.end()) return 0;
    return lpi->second;
  };

  inline const TagSet &tags(const TokID tokid)
  {
    if (tokid >= lextags.size()) return emptyset;
    return lextags[tokid];
  };

  //-- manipulation
  inline void add(const TokID tokid, const TagID tagid, const ProbT count)
  {
    //-- add tag to tagset
    if (tokid >= lextags.size()) lextags.resize(tokid+1);
    //lextags[tokid].insert(tagid);
    lextags[tokid].push_back(tagid);

    //-- add count
    IDPair lpkey(tokid,tagid);
    LexProbTable::iterator lpi = lexprobs.find(lpkey);
    if (lpi == lexprobs.end()) {
      lexprobs[lpkey] = 0;
    } else {
      lpi->second += count;
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
    bench_lookup(tokid, tags(tokid));
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

#define BENCH_CLASS LexProbsSepHV
//#define BENCH_NAME  "LexProbsSepHV-V"
//#define BENCH_ITERS 65536
#define BENCH_ITERS 131072

#define INIT_NAME "(initialize)"
#define LOOKUP_NAME "bench_lookup(tokid,tagid)"
#define GETTAGS_NAME "bench_tags(tokid)"

#define TOK_MINTAGS 1
#define TOK_MAXTAGS 10

#define NTOKENS 51277
#define NTAGS   56

/**
 * Big tagset
 */
/*
#undef NTAGS
#undef TOK_MAXTAGS
#define NTAGS 2002
#define TOK_MAXTAGS 32
*/


#define DOBENCH(iters,code) \
  for (unsigned u = 0; u < iters; u++) { code ; }


/*----------------------------------------------------------------------
 * MAIN
 *----------------------------------------------------------------------*/

int main(int argc, char **argv) {
  timeval  t1,t2;   //-- for timing info
  //ProbT    prob;    //-- for lookup
  unsigned tok, ntags, tagu, u;

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

  //-- memcheck
  fprintf(stderr, "\nDone. Memory usage? ");
  char memusg[512];
  scanf("%s", memusg);
  printf("\nMemory Usage = %s\n", memusg);

  return 0;
}
