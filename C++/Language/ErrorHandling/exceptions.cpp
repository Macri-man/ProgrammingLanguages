#include <iostream>
#include <stdexcept> // For std::runtime_error

void divide(int a, int b) {
  if (b == 0) {
    throw std::runtime_error("Division by zero error");
  }
  std::cout << "Result: " << a / b << std::endl;
}

int main() {
  try {
    divide(10, 0);
  } catch (const std::exception &e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }
  return 0;
}