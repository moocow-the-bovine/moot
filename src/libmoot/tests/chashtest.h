#include <stdio.h>
#include <vector>
#include <map>

#include <FSM.h>
#include <FSMSymSpec.h>

#include "mootTagger.h"
#include "mootTypes.h"
#include "mootLexfreqs.h"
#include "mootNgrams.h"
#include "mootHMM.h"

#ifndef _CHASHTEST_H
#define _CHASHTEST_H

using namespace std;

/*----------------------------------------------------------------------
 * Globals
 */
typedef double  ProbT;
//typedef size_t TokID;
typedef size_t TagID;

char *progname;
timeval t1, t2;

mootTagger   tgr;
mootLexfreqs lexfreqs;
mootNgrams   ngrams;
mootHMM      hmm;

/*----------------------------------------------------------------------
 * Morphology
 */
#define MORPH_FILE "moot.fst"
#define SYM_FILE   "moot.sym"

/*----------------------------------------------------------------------
 * HMM
 */
#define UNKNOWN_LEX_THRESH 1

/*----------------------------------------------------------------------
 * Bench Defines
 *----------------------------------------------------------------------*/
//-- classname to benchmark
#undef BENCH_CLASS

#define BENCH_ITERS 1
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
//#define BENCH_ITERS 4194304


/*----------------------------------------------------------------------
 * Abstract class (API spec)
 *----------------------------------------------------------------------*/
class chashtest {
public:
  chashtest(void) {};
  virtual ~chashtest(void) {};

  //-- initialization
  virtual void initialize() {};

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

#endif _CHASHTEST_H

