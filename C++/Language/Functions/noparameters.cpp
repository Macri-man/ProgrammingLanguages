#include <iostream>

void greet(); // Function declaration

int main() {
  greet(); // Function call
  return 0;
}

void greet() { // Function definition
  std::cout << "Hello, World!" << std::endl;
}