#include <iostream>
#include <vector>

int main() {
  std::vector<int> numbers = {1, 2, 3, 4, 5};

  std::cout << "Size: " << numbers.size() << std::endl;
  std::cout << "Capacity: " << numbers.capacity() << std::endl;

  numbers.resize(10); // Resize the vector to 10 elements
  std::cout << "New Size: " << numbers.size() << std::endl;

  numbers.reserve(20); // Reserve space for 20 elements
  std::cout << "New Capacity: " << numbers.capacity() << std::endl;

  return 0;
}