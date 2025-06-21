#include <stdio.h>

inline int add(int a, int b) { return a + b; }

int main() {
  int sum = add(5, 3);      // Compiler may inline the function
  printf("Sum: %d\n", sum); // Output: 8
  return 0;
}