#include <iostream>

int main() {
  int x = 10;
  auto printX = [x]() { std::cout << "Captured by value: " << x << std::endl; };

  printX();
  return 0;
}