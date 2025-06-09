#include <stdio.h>

// Recursive function to calculate factorial
int factorial(int n) {
  if (n == 0 || n == 1) {
    return 1; // Base case: factorial of 0 or 1 is 1
  } else {
    return n * factorial(n - 1); // Recursive call
  }
}

int main() {
  int num = 5;
  printf("Factorial of %d is %d\n", num, factorial(num)); // Output: 120
  return 0;
}