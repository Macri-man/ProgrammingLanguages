#include <iostream>

int multiply(int a, int b);          // Function declaration
double multiply(double a, double b); // Function declaration

int main() {
  std::cout << multiply(5, 3) << std::endl;     // Calls the int version
  std::cout << multiply(2.5, 4.0) << std::endl; // Calls the double version
  return 0;
}

int multiply(int a, int b) { // Function definition
  return a * b;
}

double multiply(double a, double b) { // Function definition
  return a * b;
}