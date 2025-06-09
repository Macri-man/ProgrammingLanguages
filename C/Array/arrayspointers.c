#include <stdio.h>

int main() {
  int arr[] = {1, 2, 3, 4, 5};
  int *ptr = arr; // Pointer to the first element of the array

  // Access elements using pointer
  for (int i = 0; i < 5; i++) {
    printf("Element %d: %d\n", i, *(ptr + i)); // Pointer arithmetic
  }

  return 0;
}