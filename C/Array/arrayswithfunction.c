#include <stdio.h>

void printArray(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
  }
}

int main() {
  int arr[] = {10, 20, 30, 40, 50};
  int size = sizeof(arr) / sizeof(arr[0]); // Calculate size of the array

  // Pass array to the function
  printArray(arr, size);

  return 0;
}