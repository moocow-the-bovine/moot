#include <stdio.h>
#include <ctype.h>
#include <locale.h>

int main (int argc, char **argv) {
  char *s;
  int i;

  setlocale(LC_ALL, ""); /* this is key! */

  while (--argc > 0) {
    s = *(++argv);
    for (i = 0; s[i] != '\0'; i++) {
      printf("isalpha(%c) = %s\n", s[i], isalpha(s[i]) ? "yup" : "nope"); /* works with user's LANG set */
      printf("ispunct(%c) = %s\n", s[i], ispunct(s[i]) ? "yup" : "nope");
      printf("isspace(%c) = %s\n", s[i], isspace(s[i]) ? "yup" : "nope");
      printf("\n");
    }
    printf("--\n");
  }
  return 0;
}
