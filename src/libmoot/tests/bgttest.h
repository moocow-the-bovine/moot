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
//typedef size_t TokID;
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
//#define BENCH_ITERS 32768
//#define BENCH_ITERS 65536
//#define BENCH_ITERS 131072
//#define BENCH_ITERS 262144
//#define BENCH_ITERS 524288
//#define BENCH_ITERS 1048576
//#define BENCH_ITERS 2097152
#define BENCH_ITERS 4194304

#define INIT_NAME  "(initialize)"
#define BENCH_NAME "(lookup)"
//#define BENCH_ITER_NAME  "(iter)"

//-- choose tagset
//#define LARGE_TAGSET

#ifndef LARGE_TAGSET
# define NTAGS      56
# define MIN_NPTAGS  7
# define MAX_NPTAGS 49
#else /* LARGET_TAGSET */
# define NTAGS       2002
# define MIN_NPTAGS   101
# define MAX_NPTAGS  1001
#endif /* LARGE_TAGSET */

/*----------------------------------------------------------------------
 * Abstract class (API spec)
 *----------------------------------------------------------------------*/
class bgTest {
public:
  bgTest(void) {};
  virtual ~bgTest(void) {};

  //-- initialization
  virtual void initialize(size_t ntags=0) {};

  //-- lookup
  inline const ProbT lookup(const TagID ptagid, const TagID tagid) const;

  //-- manipulation
  void add(const TagID tokid, const TagID tagid, const ProbT count);

  //-- bench lookup
  void bench_lookup(const size_t iters);

  //-- bench ptag-iteration
  //void bench_iter(const size_t iters);
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
