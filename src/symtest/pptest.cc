#include <stdio.h>

/*
 * MAIN
 */
int main (int argc, char **argv) {
  int x, *xp, **xpp;
  x = 42;
  xp = &x;
  xpp = &xp;

  printf("x=%d, *xp=%d, **xpp=%d\n",
	 x, *xp, **xpp);
  return 0;
}
