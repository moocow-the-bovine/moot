#include <stdio.h>

#include <vector>

using namespace std;

int main (int argc, char **argv) {
  vector<int> iv;

  printf("\nnew vector<int>():\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());

  iv.resize(1);
  printf("\nresize(1):\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());
  printf("iv[0] = %g\n", (double)(iv[0]));

  iv.push_back(42);
  printf("\npush_back(42):\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());

  iv.reserve(512);
  printf("\nreserve(512):\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());

  iv.clear();
  printf("\npost-clear():\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());
  
  return 0;
}
