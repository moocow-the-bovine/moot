#include <stdio.h>
#include <string>
#include <list>

using namespace std;

int main (void) {
  list<string> l;
  l.push_back("foo");
  l.push_back("bar");

  list<string>::const_iterator li1 = l.begin();

  printf("&(*li1) %c= &(l.front())\n",
	 &(*li1) == &(l.front()) ? '=' : '!');

  return 0;
}
