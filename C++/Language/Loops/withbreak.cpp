#include <iostream>

int main() {
  for (int i = 0; i < 10; ++i) {
    if (i == 5) {
      std::cout << "Breaking out of the loop at i = " << i << std::endl;
      break;
    }
    std::cout << "Iteration: " << i << std::endl;
  }

  int i = 0;
  while (true) { // Infinite loop
    if (i == 5) {
      std::cout << "Breaking out of the loop at i = " << i << std::endl;
      break;
    }
    std::cout << "Iteration: " << i << std::endl;
    ++i;
  }

  int i = 0;
  do {
    if (i == 5) {
      std::cout << "Breaking out of the loop at i = " << i << std::endl;
      break;
    }
    std::cout << "Iteration: " << i << std::endl;
    ++i;
  } while (i < 10);

  return 0;
}