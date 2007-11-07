#include <mootHMM.h>
#include <stdio.h>
using namespace moot;

#define PTR 1

#if !defined(PTR)
mootHMM hmm;
#endif

int main(void) {

#if defined(PTR)
  mootHMM *hmmp = NULL;
  hmmp = new mootHMM();
  fprintf(stderr, "tokids.size=%u, tagids.size=%u\n", hmmp->tokids.size(), hmmp->tagids.size());
  fprintf(stderr, "tokids.ptr =%p, tagids.ptr =%p\n", &(hmmp->tokids), &(hmmp->tagids));
  fflush(stderr);
  delete(hmmp);
#else
  fprintf(stderr, "tokids.size=%u, tagids.size=%u\n", hmm.tokids.size(), hmm.tagids.size());
  fflush(stderr);
#endif

  return 0;
}
