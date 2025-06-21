#include <stdio.h>
#include <stdlib.h> // Required for malloc and free

int main() {
  int n;
  printf("Enter the number of elements: ");
  scanf("%d", &n);

  // Dynamically allocate memory for an array of n integers
  int *arr = (int *)malloc(n * sizeof(int));

  if (arr == NULL) {
    printf("Memory allocation failed\n");
    return 1;
  }

  // Initialize the array
  for (int i = 0; i < n; i++) {
    arr[i] = i + 1; // Assign values
  }

  // Print the array
  for (int i = 0; i < n; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
  }

  free(arr); // Free the allocated memory

  return 0;
}