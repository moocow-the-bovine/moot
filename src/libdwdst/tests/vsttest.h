/* -*- Mode: C++ -*- */
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include <vector>
#include <deque>
#include <list>
#include <slist>
#include <hash_map>
#include <map>
#include <set>
#include <hash_set>

using namespace std;

/*----------------------------------------------------------------------
 * globals
 *----------------------------------------------------------------------*/
typedef float  ProbT;
typedef size_t TokID;
typedef size_t TagID;

char *progname;
timeval t1, t2;

/*----------------------------------------------------------------------
 * Bench Defines
 *----------------------------------------------------------------------*/
//-- classname to benchmark
#undef BENCH_CLASS
//#define BENCH_ITERS 128
//#define BENCH_ITERS 1024
//#define BENCH_ITERS 8192
#define BENCH_ITERS 32768
//#define BENCH_ITERS 65536
//#define BENCH_ITERS 131072

//#define BENCH_LONG_ITERS 128
//#define BENCH_LONG_ITERS 1024
#define BENCH_LONG_ITERS 4096
//#define BENCH_LONG_ITERS 32768
//#define BENCH_LONG_ITERS 65536
//#define BENCH_LONG_ITERS 131072

#define INIT_NAME  "(initialize)"
#define BENCH_NAME "normal: (clear)+(n*step)+(bestpath_iter)"
#define BENCH_LONG_NAME  "long: (clear)+(n*step)+(bestpath_iter)"

//-- choose tagset
//#define LARGE_TAGSET

#ifndef LARGE_TAGSET
# define NTAGS       56
# define TOK_MINTAGS 1
# define TOK_MAXTAGS 10
#else /* LARGET_TAGSET */
# define NTAGS       2002
# define TOK_MINTAGS 1
# define TOK_MAXTAGS 32
#endif /* LARGE_TAGSET */

//-- initial sentence length
#define SLEN_INIT 0
//#define SLEN_INIT 32
//#define SLEN_INIT 128

//-- sentence length: range
#define SLEN_MIN 2
#define SLEN_MAX 32

//-- sentence length: outliers
#define SLEN_LONG_MIN 64
#define SLEN_LONG_MAX 128
//#define SLEN_LONG_MAX 1024

/*----------------------------------------------------------------------
 * Sentence generation
 *----------------------------------------------------------------------*/
class vstInSent {
  typedef vector<TokID>  TokVec;
  typedef vector<TagID>  TagSet;
  typedef vector<TagSet> TagSetVec;

public:
  TokVec    tokids;
  TagSetVec tagsets;

public:
  //-- constructor
  vstInSent(size_t sentlen=0) {
    if (sentlen) reserve(sentlen);
  };

  //-- destructor
  ~vstInSent(void)
  {
    tokids.clear();
    tagsets.clear();
  };

  //-- reserve
  inline void reserve(size_t sentlen)
  {
    tokids.reserve(sentlen);
    tagsets.reserve(sentlen);
  };

  //-- size
  inline size_t size(void) const { return tokids.size(); };

  //-- clear
  inline void clear(void) { tokids.clear(); tagsets.clear(); };

  //-- generate
  inline void generate(size_t sentlen=0)
  {
    clear();
    if (!sentlen) sentlen = SLEN_MIN + (rand()%(SLEN_MAX-SLEN_MIN));
    reserve(sentlen);

    size_t ntags, i, j;
    for (i = 0; i < sentlen; i++) {
      tokids.push_back(i);

      ntags = TOK_MINTAGS + (rand()%(TOK_MAXTAGS-TOK_MINTAGS));
      tagsets.push_back(TagSet(ntags));
      TagSet &tagset = tagsets.back();
      for (j = 0; j < ntags; j++) {
	tagset[j] = rand()%NTAGS;
      }
    }
  };
};


/*----------------------------------------------------------------------
 * Abstract class (API spec)
 *----------------------------------------------------------------------*/
class vstTest {
public:
  vstTest(void) {};
  virtual ~vstTest(void) {};

  //-- initialization
  virtual void initialize(size_t ntags=0, size_t sentlen=0) {};

  //-- (clear)+(n*step)+(bestpath_iter)
  virtual void bench(const vstInSent &sent) {};
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
