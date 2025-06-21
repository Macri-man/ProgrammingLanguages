#include <iostream>

int main() {
  const int SIZE = 10; // Constant size
  int numbers[SIZE];

  for (int i = 0; i < SIZE; ++i) {
    numbers[i] = i * 2;
    std::cout << "numbers[" << i << "] = " << numbers[i] << std::endl;
  }
  return 0;
}