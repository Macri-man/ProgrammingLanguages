#include <iostream>

int main() {
  int numbers[5]; // Declaration of an array of 5 integers
  numbers[0] = 1; // Initializing elements
  numbers[1] = 2;
  numbers[2] = 3;
  numbers[3] = 4;
  numbers[4] = 5;

  // Accessing elements
  for (int i = 0; i < 5; ++i) {
    std::cout << "numbers[" << i << "] = " << numbers[i] << std::endl;
  }

  int numbers2[5] = {1, 2, 3, 4, 5}; // Initialization with values

  for (int i = 0; i < 5; ++i) {
    std::cout << "numbers[" << i << "] = " << numbers2[i] << std::endl;
  }

  return 0;
}