#include <iostream>
#include <memory>


int main() {
  std::unique_ptr<int> ptr = std::make_unique<int>(10);
  std::cout << "Value: " << *ptr << std::endl; // Outputs: Value: 10

  // No need to explicitly delete; memory is managed automatically
  return 0;
}