#include <algorithm> // For algorithms
#include <iostream>
#include <vector>


int main() {
  std::vector<int> numbers = {1, 2, 3, 4, 5};

  // Using std::sort to sort the vector
  std::sort(numbers.begin(), numbers.end());

  // Using std::reverse to reverse the vector
  std::reverse(numbers.begin(), numbers.end());

  for (int num : numbers) {
    std::cout << num << " "; // Outputs: 5 4 3 2 1
  }
  std::cout << std::endl;

  return 0;
}