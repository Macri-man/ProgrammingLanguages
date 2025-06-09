#include <iostream>

int main() {
  auto multiply = [](double a, double b) -> double { return a * b; };

  std::cout << "Product: " << multiply(4.5, 3.2) << std::endl;
  return 0;
}