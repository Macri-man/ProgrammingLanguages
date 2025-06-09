#include <stdio.h>

// Function declaration
int add(int a, int b); // Declares the function

int main() {
  int x = 10, y = 20;
  int sum = add(x, y);      // Call the function
  printf("Sum: %d\n", sum); // Output: 30
  return 0;
}

// Function definition
int add(int a, int b) {
  return a + b; // Return the sum of a and b
}