#include <stdio.h>

#include <vector>

using namespace std;

int main (int argc, char **argv) {
  vector<int> iv;

  printf("\nnew vector<int>():\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());

  iv.reserve(512);
  printf("\nreserve(512):\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());

  iv.clear();
  printf("\npost-clear():\n");
  printf("iv.size()=%u ; iv.capacity()=%u\n", iv.size(), iv.capacity());
  
  return 0;
}
