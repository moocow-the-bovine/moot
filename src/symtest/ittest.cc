#include <stdio.h>
#include <string>
#include <set>
#include <vector>

using namespace std;

/*-----------------------------------------------------------------
 * Globals
 */

typedef set<string> tagSet;
typedef vector<tagSet::iterator> tagSetIterVector;

tagSet alltags;
tagSetIterVector tagIters;
int kmax = 3;

/*-----------------------------------------------------------------
 * Iteration Utilities
 */
tagSetIterVector &tagIters_init(int len) {
  // -- initialize
  tagIters.clear();
  //tagIters.reserve(len);
  for (int j = 0; j < len; j++) {
    tagIters.push_back(alltags.begin());
  }
  return tagIters;
}

tagSetIterVector &tagIters_next(void) {
  for (int j = tagIters.size()-1; j >= 0; j--) {
    if (++tagIters[j] != alltags.end() || j == 0) {
      // -- just step this position
      break;
    }
    // -- reset this position and increment the next highest
    tagIters[j] = alltags.begin();
  }
  return tagIters;
}

bool tagIters_end() { return tagIters[0] == alltags.end(); }


/*-----------------------------------------------------------------
 * Test Functions
 */
void iter_test() {
  int len;
  for (len = 1; len <= kmax; len++) {
    for (tagIters_init(len); !tagIters_end(); tagIters_next()) {
      // -- report
      printf("Iteration [len=%d, tagIters.size()=%d]: ", len, tagIters.size());
      for (tagSetIterVector::iterator tsi = tagIters.begin(); tsi != tagIters.end(); tsi++)
	{
	  printf("%s,", (*tsi)->c_str());
	}
      printf("\n");
    }
  }
}


/*-----------------------------------------------------------------
 * MAIN
 */
int main (int argc, char **argv) {
  // -- initialize alltags
  char *progname = *argv;
  while (--argc > 0) {
    string s = *(++argv);
    alltags.insert(s);
  }
  // -- report
  printf("%s: iterating over tagset {", progname);
  for (tagSet::iterator i = alltags.begin(); i != alltags.end(); i++) {
    printf("%s,", i->c_str());
  }
  printf("} of size %d\n", alltags.size());

  // -- do iteration
  iter_test();

  return 0;
}
