#include <stdio.h>
#include <stdlib.h>

struct ilist {
  int i;
  struct ilist *next;
};

int main (int argc, char **argv) {
  char *progname = *argv;

  ilist
    *i1 = new ilist(),
    *i2 = new ilist(),
    *i3 = new ilist();

  i1->i = 1;
  i2->i = 2;
  i3->i = 3;

  i1->next = i2;
  i2->next = i3;
  i3->next = NULL;

  ilist *il, *il_next, *trash_il = NULL;
  for (il = i1; il != NULL; il = il_next) {
    il_next = il->next;
    il->next = trash_il;
    trash_il = il;
  }

  return 0;
}
