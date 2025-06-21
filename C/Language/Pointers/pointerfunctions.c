#include <stdio.h>

void increment(int *p) {
  (*p)++; // Dereference the pointer and increment the value
}

int main() {
  int x = 10;
  increment(&x);                                 // Pass the address of x
  printf("Value of x after increment: %d\n", x); // Output: 11

  return 0;
}