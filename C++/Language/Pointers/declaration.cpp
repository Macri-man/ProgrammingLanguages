#include <iostream>

int main() {
  int value = 10;
  int *ptr = &value;

  std::cout << "Value: " << *ptr << std::endl; // Outputs: Value: 10
  return 0
}