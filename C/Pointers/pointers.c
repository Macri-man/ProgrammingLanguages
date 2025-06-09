#include <stdio.h>

int main() {
  int x = 10;    // Regular variable
  int *ptr = &x; // Pointer variable, stores the address of x

  // Print the value of x, address of x, and the value stored at the pointer
  printf("Value of x: %d\n", x);
  printf("Address of x: %p\n", &x);
  printf("Value stored at ptr (dereferencing): %d\n", *ptr);

  return 0;
}