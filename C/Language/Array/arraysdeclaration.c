#include <stdio.h>

int main() {
  int arr[5]; // Declare an integer array with 5 elements

  // Initialize the array
  arr[0] = 10;
  arr[1] = 20;
  arr[2] = 30;
  arr[3] = 40;
  arr[4] = 50;

  // Access and print array elements
  for (int i = 0; i < 5; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
  }

  int arr2[5] = {10, 20, 30, 40, 50}; // Declare and initialize the array

  for (int i = 0; i < 5; i++) {
    printf("arr[%d] = %d\n", i, arr2[i]);
  }

  return 0;
}