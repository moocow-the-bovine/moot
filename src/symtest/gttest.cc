#include <stdio.h>

/*
 * MAIN
 */
int main (int argc, char **argv) {
    int i;
    for (i = 0; i < 5; i++) {
	if (i % 2 == 0) {
	    goto even;
	} else {
	    printf("odd: %d\n", i);
	    continue;
	}
    even:
	printf("even: %d\n", i);
    }
    return 0;
}
