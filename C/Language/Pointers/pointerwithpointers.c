#include <stdio.h>

int main() {
  int x = 10;
  int *ptr = &x;     // Pointer to an int
  int **ptr2 = &ptr; // Pointer to a pointer

  printf("Value of x: %d\n", **ptr2); // Dereference twice to get the value of x

  return 0;
}