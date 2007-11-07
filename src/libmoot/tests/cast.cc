#include <iostream>

int main (void) {
  unsigned int u = static_cast<unsigned int>(-1);
  double       d = static_cast<double>(u);

  std::cout
    << "u=" << u
    << "; d=" << d
    << "\n";
    

  return 0;
}
