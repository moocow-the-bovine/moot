#include <stdio.h>
#include <list>

using namespace std;

typedef list<int> myList;

int main (void) {
  myList l1;
  myList l2;

  l1.push_back(42);
  l1.push_back(24);

  int *ip = &(l1.front());

  l2.splice(l2.begin(), l1, l1.begin(), l1.end());
  printf("ip %c= l2.front()\n", ip == &(l2.front()) ? '=' : '!');

  return 0;
}
