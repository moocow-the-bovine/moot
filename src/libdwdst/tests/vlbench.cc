#include <stdio.h>

#include <vector>
#include <list>
#include <slist>

using namespace std;

/*----------------------------------------------
 * Types
 */
typedef vector<int> intVec;
typedef list<int>   intList;
typedef slist<int>  intSlist;

#define ITERS 32768
#define MAXLENGTH 32
unsigned maxlength = MAXLENGTH;  //-- in a variable

/*----------------------------------------------
 * Globals
 */
intVec   ivs[MAXLENGTH];
intList  ils[MAXLENGTH];
intSlist isls[MAXLENGTH];

/*----------------------------------------------
 * Benchmark: preparation
 */
void prepare_globals(void) {
  for (int len = 0; len < MAXLENGTH; len++) {
    for (int i = 0; i < len; i++) {
      ivs[len].push_back(i);
      ils[len].push_back(i);

      //isls[len].push_back(i);
      isls[len].push_front(i); //-- reverse order for slist!
    }
  }
}


/*----------------------------------------------
 * Benchmark: vector
 */
void bench_vector(unsigned iters) {
  while (--iters > 0) {
    for (unsigned len = 0; len < maxlength; len++) {
      //-- copy, push, delete
      intVec *iv2 = new intVec(ivs[len]);
      iv2->push_back(len);
      delete iv2;
    }
  }
}

/*----------------------------------------------
 * Benchmark: list
 */
void bench_list(unsigned iters) {
  while (--iters > 0) {
    for (unsigned len = 0; len < maxlength; len++) {
      //-- copy, push, delete
      intList *il2 = new intList(ils[len]);
      il2->push_back(len);
      delete il2;
    }
  }
}

/*----------------------------------------------
 * Benchmark: slist
 */
void bench_slist(unsigned iters) {
  while (--iters > 0) {
    for (unsigned len = 0; len < maxlength; len++) {
      //-- copy, push, delete
      intSlist *isl2 = new intSlist(isls[len]);
      //isl2->push_back(len);
      isl2->push_front(len); //-- reverse order for slist!
      delete isl2;
    }
  }
}


/*----------------------------------------------
 * MAIN
 */
int main (int argc, char **argv) {
  fprintf(stderr, "%s: Preparing... ", *argv);
  prepare_globals();
  fprintf(stderr, " done.\n");

  fprintf(stderr, "%s: Benchmarking %u iterations of vector<int>::copy,push,delete...",
	  *argv, ITERS);
  bench_vector(ITERS);
  fprintf(stderr, " done.\n");

  fprintf(stderr, "%s: Benchmarking %u iterations of list<int>::copy,push,delete...",
	  *argv, ITERS);
  bench_list(ITERS);
  fprintf(stderr, " done.\n");

  fprintf(stderr, "%s: Benchmarking %u iterations of slist<int>::copy,push,delete...",
	  *argv, ITERS);
  bench_slist(ITERS);
  fprintf(stderr, " done.\n");

  return 0;
}
