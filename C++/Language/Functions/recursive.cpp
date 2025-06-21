#include <iostream>

int factorial(int n); // Function declaration

int main() {
  std::cout << "Factorial of 5: " << factorial(5) << std::endl;
  return 0;
}

int factorial(int n) { // Function definition
  if (n <= 1)
    return 1;
  else
    return n * factorial(n - 1);
}