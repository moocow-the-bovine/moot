#include <stdio.h>

#include <hash_map>

using namespace std;

typedef hash_map<int,int> iihash;

int main (void) {
  iihash iih;

  iih[420] = 24;
  iih[24]  = 7;

  int *iih_420 = &(iih[420]);
  int *iih_24  = &(iih[24]);
  int *iih_7   = &(iih[7]);

  printf("iih_420: %d\n", *iih_420);
  printf("iih_24: %d\n", *iih_24);
  printf("iih_7: %d\n", *iih_7);

  return 0;
}
