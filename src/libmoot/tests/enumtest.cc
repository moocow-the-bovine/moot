#include <stdio.h>

enum testEnumE {
  foo=1,
  bar=2,
  baz=3,
  bonk=4
};
typedef testEnumE testEnumT;

int main (void) {
  int i = 5;
  testEnumT e = (testEnumT)i;

  printf("i=%d ; e=%d ; (testEnumT)i=%d\n", i, e, (testEnumT)i);

  return 0;
}
