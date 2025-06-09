#include <stdio.h>

// Function that adds two numbers
int add(int a, int b) { return a + b; }

int main() {
  // Declare a function pointer
  int (*funcPtr)(int, int) = &add;

  // Call the function using the pointer
  int result = funcPtr(5, 3);
  printf("Result: %d\n", result); // Output: 8

  return 0;
}