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

typedef set<TagID> TagSet;

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
 * separate storage : hash_map, vector
 *----------------------------------------------------------------------*/
class LexProbsSepH : public LexProbs
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
  LexProbsSepH(void) {};
  ~LexProbsSepH(void) {};

  //-- lookup
  inline const ProbT lookup(TokID tokid, TagID tagid) const
  {
    LexProbTable::const_iterator lpi = lexprobs.find(IDPair(tokid,tagid));
    if (lpi == lexprobs.end()) return 0;
    return lpi->second;
  };

  inline const TagSet &tags(TokID tokid)
  {
    if (tokid >= lextags.size()) return emptyset;
    return lextags[tokid];
  };

  //-- manipulation
  inline void add(TokID tokid, TagID tagid, ProbT count)
  {
    //-- add tag to tagset
    if (tokid >= lextags.size()) lextags.resize(tokid+1);
    lextags[tokid].insert(tagid);

    //-- add count
    IDPair lpkey(tokid,tagid);
    LexProbTable::iterator lpi = lexprobs.find(lpkey);
    if (lpi == lexprobs.end()) {
      lexprobs[lpkey] = 0;
    } else {
      lpi->second += count;
    }
  };
};



/*----------------------------------------------------------------------
 * Recursive vector-hash : vector<hash_map<TagID,ProbT> >
 *----------------------------------------------------------------------*/
class LexProbsRecVH : public LexProbs
{
public:
  /** Type for lookup table: TokID->TagID->ProbT. */
  typedef vector<hash_map<TagID,ProbT> > ProbTable;

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
  LexProbsSepH(void) {};
  ~LexProbsSepH(void) {};

  //-- lookup
  inline const ProbT lookup(TokID tokid, TagID tagid) const
  {
    LexProbTable::const_iterator lpi = lexprobs.find(IDPair(tokid,tagid));
    if (lpi == lexprobs.end()) return 0;
    return lpi->second;
  };

  inline const TagSet &tags(TokID tokid)
  {
    if (tokid >= lextags.size()) return emptyset;
    return lextags[tokid];
  };

  //-- manipulation
  inline void add(TokID tokid, TagID tagid, ProbT count)
  {
    //-- add tag to tagset
    if (tokid >= lextags.size()) lextags.resize(tokid+1);
    lextags[tokid].insert(tagid);

    //-- add count
    IDPair lpkey(tokid,tagid);
    LexProbTable::iterator lpi = lexprobs.find(lpkey);
    if (lpi == lexprobs.end()) {
      lexprobs[lpkey] = 0;
    } else {
      lpi->second += count;
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
  printf("Benchmarked %u iterations of '%s':\n", iters, name);
  printf("   Elapsed : %g sec\n", elapsed);
  printf("     Rate  : %g iters/sec\n", (double)iters / elapsed);
}

/*----------------------------------------------------------------------
 * Bench Defines
 *----------------------------------------------------------------------*/

#define BENCH_CLASS LexProbsSepH
#define BENCH_NAME  "LexProbsSepH"
#define BENCH_ITERS 65536

#define INIT_NAME ": Init()"
#define GETTAGS_NAME ": GetTags(tokid)"
#define LOOKUP_NAME ": Lookup(tokid,tagid)"

#define TOK_MINTAGS 1
#define TOK_MAXTAGS 10

#define NTOKENS 51277
#define NTAGS   56

#define DOBENCH(iters,code) \
  for (unsigned u = 0; u < iters; u++) { code ; }

/*----------------------------------------------------------------------
 * MAIN
 *----------------------------------------------------------------------*/

int main(void) {
  timeval  t1,t2;   //-- for timing info
  ProbT    prob;    //-- for lookup
  unsigned tok, ntags, tagu, u;

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
  benchinfo(BENCH_NAME INIT_NAME, 1, t1, t2);

  //-- lookup
  gettimeofday(&t1, NULL);
  for (u = 0; u < BENCH_ITERS; u++) {
    prob = lp.lookup(rand()%NTOKENS,rand()%NTAGS);
  }
  gettimeofday(&t2, NULL);
  benchinfo(BENCH_NAME LOOKUP_NAME, BENCH_ITERS, t1, t2);

  //-- tag-lookup
  gettimeofday(&t1, NULL);
  for (u = 0; u < BENCH_ITERS; u++) {
    tok = rand()%NTOKENS;
    const TagSet &tagset = lp.tags(rand()%NTOKENS);
    for (TagSet::const_iterator tsi = tagset.begin(); tsi != tagset.end(); tsi++) {
      prob = lp.lookup(tok,*tsi);
    }
  }
  gettimeofday(&t2, NULL);
  benchinfo(BENCH_NAME GETTAGS_NAME, BENCH_ITERS, t1, t2);
  

  return 0;
}
