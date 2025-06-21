#include <stdio.h>

int main() {
  int arr[] = {1, 2, 3, 4, 5}; // Size is inferred based on initialization

  printf("First element: %d\n", arr[0]); // Access first element
  printf("Last element: %d\n", arr[4]);  // Access last element

  return 0;
}