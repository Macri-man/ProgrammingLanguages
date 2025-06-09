#include <iostream>

int add(int a, int b); // Function declaration

int main() {
  int result = add(5, 3); // Function call
  std::cout << "Result: " << result << std::endl;
  return 0;
}

int add(int a, int b) { // Function definition
  return a + b;
}