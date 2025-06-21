#include <iostream>

int main() {
  for (int i = 0; i < 10; ++i) {
    if (i % 2 == 0) {
      // Skip even numbers
      continue;
    }
    std::cout << "Odd number: " << i << std::endl;
  }

  int i = 0;
  while (i < 10) {
    if (i % 2 == 0) {
      // Skip even numbers
      ++i; // Important to avoid infinite loop
      continue;
    }
    std::cout << "Odd number: " << i << std::endl;
    ++i;
  }

  int i = 0;
  do {
    if (i % 2 == 0) {
      // Skip even numbers
      ++i; // Important to avoid infinite loop
      continue;
    }
    std::cout << "Odd number: " << i << std::endl;
    ++i;
  } while (i < 10);

  return 0;
}