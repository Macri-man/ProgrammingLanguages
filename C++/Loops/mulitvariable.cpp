#include <iostream>

int main() {
  for (int i = 0, j = 10; i < 5 && j > 0; ++i, --j) {
    std::cout << "i = " << i << ", j = " << j << std::endl;
  }

  int i = 0;
  int j = 10;
  while (i < 5 && j > 0) {
    std::cout << "i = " << i << ", j = " << j << std::endl;
    ++i;
    --j;
  }

  int i = 0;
  int j = 10;
  do {
    std::cout << "i = " << i << ", j = " << j << std::endl;
    ++i;
    --j;
  } while (i < 5 && j > 0);
  return 0;
}