#include <iostream>
#include <vector>

int main() {
  std::vector<int> numbers = {1, 2, 3, 4, 5};

  numbers.pop_back();                 // Removes 5
  numbers.erase(numbers.begin() + 1); // Removes 2

  for (int num : numbers) {
    std::cout << num << " "; // Outputs: 1 3 4
  }
  std::cout << std::endl;

  numbers.clear(); // Clears all elements

  return 0;
}