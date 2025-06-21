#include <iostream>
#include <vector>

int main() {
  std::vector<int> numbers; // Declaring an empty vector of integers

  // Initializing a vector with values
  std::vector<int> initializedNumbers = {1, 2, 3, 4, 5};

  // Adding elements to a vector
  numbers.push_back(10); // Adds 10 to the end of the vector
  numbers.push_back(20);

  // Accessing elements
  std::cout << "First element: " << numbers[0] << std::endl;
  std::cout << "Second element: " << numbers.at(1)
            << std::endl; // .at() provides bounds checking

  return 0;
}