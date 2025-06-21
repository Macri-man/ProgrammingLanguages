#include <stdio.h>

int main() {
  int arr[] = {10, 20, 30, 40, 50};
  int *ptr = arr; // Points to the first element of the array

  // Access array elements using pointer arithmetic
  printf("First element: %d\n", *ptr);
  printf("Second element: %d\n",
         *(ptr + 1)); // Moves to the next int (4 bytes forward)
  printf("Third element: %d\n", *(ptr + 2));

  return 0;
}