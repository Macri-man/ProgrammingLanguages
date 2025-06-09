#include <stdio.h>

int square(int x) {
  return x * x; // Return the square of x
}

void printMessage() { printf("Hello, World!\n"); }

int main() {
  int result = square(5);              // Call the function
  printf("Square of 5: %d\n", result); // Output: 25

  printMessage(); // Call the function
  return 0;
}