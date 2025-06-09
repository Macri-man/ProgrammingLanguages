#include <iostream>
#include <vector>

int main() {
  std::vector<int> numbers = {1, 2, 3, 4, 5};

  // Using a traditional for loop
  for (size_t i = 0; i < numbers.size(); ++i) {
    std::cout << numbers[i] << " ";
  }
  std::cout << std::endl;

  // Using a range-based for loop
  for (int num : numbers) {
    std::cout << num << " ";
  }
  std::cout << std::endl;

  // Using iterators
  for (std::vector<int>::iterator it = numbers.begin(); it != numbers.end();
       ++it) {
    std::cout << *it << " ";
  }
  std::cout << std::endl;

  return 0;
}