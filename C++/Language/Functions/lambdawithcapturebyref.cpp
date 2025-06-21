#include <iostream>

int main() {
  int x = 10;
  auto modifyX = [&x]() {
    x += 5;
    std::cout << "Modified x: " << x << std::endl;
  };

  modifyX();
  std::cout << "x in main: " << x << std::endl;
  return 0;
}