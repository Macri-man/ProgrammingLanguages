#include <iostream>

int main() {
  int x = 0;
  if (x > 0) {
    std::cout << "x is positive." << std::endl;
  } else if (x < 0) {
    std::cout << "x is negative." << std::endl;
  } else {
    std::cout << "x is zero." << std::endl;
  }
  return 0;
}