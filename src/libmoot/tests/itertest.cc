#include <stdio.h>
#include <vector>

using namespace std;

typedef vector<int> intVec;

int main (int argc, char **argv) {
  intVec iv;

  printf("iv.size()=%u ; iv.capacity=%u\n", iv.size(), iv.capacity());
  iv.push_back(0);

  printf("\n");
  printf("push_back(0)\n");
  printf("iv.size()=%u ; iv.capacity=%u\n", iv.size(), iv.capacity());

  iv.push_back(1);
  iv.push_back(2);
  iv.push_back(3);

  printf("\niter()\n");
  intVec::iterator ivi_best;
  for (intVec::iterator ivi = iv.begin(); ivi != iv.end(); ivi++) {
    if (*ivi %2 == 0) {
      ivi_best = ivi;
    }
  }
  printf("*ivi_best = %d\n", *ivi_best);
  return 0;
}
