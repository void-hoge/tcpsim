#include <iostream>

int main() {
  int w = 0;
  int limit1 = 100000;
  int limit2 = 150000;
  for (long long i = 0; i < 1<<10; i++) {
    std::cout << w << std::endl;
    if (w > limit1) {
      w = w*3/4;
    }else {
      w += 1000;
    }
  }
  for (long long i = 0; i < 1<<10; i++) {
    std::cout << w << std::endl;
    if (w > limit2) {
      w = w*3/4;
    }else {
      w += 1000;
    }
  }
  return 0;
}
