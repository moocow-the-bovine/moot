#include <float.h>
#include <stdio.h>

int main (void) {
  printf("DBL_MIN = %g\n", DBL_MIN);
  printf("DBL_MAX = %g\n", DBL_MAX);
  printf("-DBL_MAX = %g\n", -DBL_MAX);

  if (DBL_MIN < inf) {
    printf("DBL_MIN < inf\n");
  } else {
    printf("DBL_MIN >= inf\n");
  }
  return 0;
}
