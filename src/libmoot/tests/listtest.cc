#include <stdio.h>
#include <list>
#include <string>

using namespace std;

class aClass {
public:
  int a_x;
  aClass(int x) : a_x(x)
  {
    printf("aClass::aClass() called: [x=%d]\n", a_x);
  };
  ~aClass(void)
  {
    printf("aClass::~aClass() called: [x=%d]\n", a_x);
  };
};
typedef list<aClass> aList;

int main (void) {
  aList l;

  printf("\n--- push_back()\n");
  l.push_back(aClass(0));
  l.push_back(aClass(1));

  printf("\n--- clear()\n");
  l.clear();

  printf("\n--- end-of-main()\n");

  return 0;
}
