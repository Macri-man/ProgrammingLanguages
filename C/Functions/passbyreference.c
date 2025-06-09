#include <stdio.h>

void increment(int *x) {
  (*x)++; // Dereference the pointer to modify the original value
}

int main() {
  int a = 10;
  increment(&a);                 // Pass by reference
  printf("Value of a: %d\n", a); // Output: 11
  return 0;
}