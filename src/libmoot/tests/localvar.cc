#include <stdio.h>

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

int main (void) {
  int i;
  for (i = 0; i < 3; i++) {
    printf("--- iter %d\n", i);
    aClass ac(i);
    printf("--- /iter %d\n", i);
  }
  return 0;
}
