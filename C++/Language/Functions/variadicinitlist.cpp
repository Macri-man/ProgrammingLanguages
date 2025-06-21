#include <initializer_list>
#include <iostream>


void printNumbers(std::initializer_list<int> nums) {
  for (int num : nums) {
    std::cout << num << " ";
  }
  std::cout << std::endl;
}

int main() {
  printNumbers({1, 2, 3});
  printNumbers({10, 20, 30, 40, 50});
  return 0;
}