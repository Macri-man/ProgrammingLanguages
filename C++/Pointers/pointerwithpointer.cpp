#include <iostream>

int main() {
  int value = 10;
  int *ptr = &value;
  int **ptr_to_ptr = &ptr;

  std::cout << "Value: " << **ptr_to_ptr << std::endl; // Outputs: Value: 10

  return 0
}