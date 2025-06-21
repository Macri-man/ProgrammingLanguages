#include <stdio.h>

void increment(int x) {
  x++; // Only modifies the local copy of x
}

int main() {
  int a = 10;
  increment(a);                  // Pass by value
  printf("Value of a: %d\n", a); // Output: 10 (no change)
  return 0;
}