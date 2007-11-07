#include <mootHMM.h>
#include <stdio.h>
using namespace moot;

#define PTR 1
//#define STR 1

#if !defined(STR) && !defined(PTR)
mootHMM hmm;
#endif

int main(void) {

#if defined(STR)
  std::string s0("@UNKNOWN");
  std::string s1 = "@UNKNOWN";
  fprintf(stderr, "s0.size=%d, s0.capacity=%d, s0.npos=%d\n", s0.size(), s0.capacity(), s0.npos);
  fprintf(stderr, "s1.size=%d, s1.capacity=%d, s1.npos=%d\n", s1.size(), s1.capacity(), s1.npos);

#elif defined(PTR)
  mootHMM *hmmp = NULL;
  hmmp = new mootHMM();
  delete(hmmp);
#else
  fprintf(stderr, "tokids.size=%u, tagids.size=%u\n", hmm.tokids.size(), hmm.tagids.size());
  fflush(stderr);
#endif

  return 0;
}
