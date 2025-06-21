#include <iostream>

int main() {
  int numbers[5] = {1, 2, 3, 4, 5};
  int *ptr = numbers; // Pointer to the first element of the array

  // Accessing elements through the pointer
  for (int i = 0; i < 5; ++i) {
    std::cout << "numbers[" << i << "] = " << *(ptr + i) << std::endl;
  }
  return 0;
}