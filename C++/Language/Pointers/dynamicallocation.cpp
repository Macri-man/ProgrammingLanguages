#include <iostream>

int main() {
  int *ptr = new int; // Allocate memory for an integer
  *ptr = 20;          // Assign a value to the allocated memory

  std::cout << "Value: " << *ptr << std::endl; // Outputs: Value: 20

  delete ptr; // Deallocate the memory
  return 0
}