#include <stdio.h>
#include <unistd.h>

int main (void) {
  char c = '|';
  fputc('|',stderr);
  while (1) {
    sleep(1);
    fputc('\b', stderr);
    switch (c) {
    case '|':
      c = '/';
      break;
    case '/':
      c = '-';
      break;
    case '-':
      c = '\\';
      break;
    case '\\':
      c = '|';
      break;
    default:
      c = '|';
    }
    fputc(c,stderr);
    //fputc('\n', stderr);
  }
}
