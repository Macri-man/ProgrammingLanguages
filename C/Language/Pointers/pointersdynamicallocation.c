#include <stdio.h>
#include <stdlib.h> // For malloc and free

int main() {
  int *ptr = (int *)malloc(5 * sizeof(int)); // Allocate memory for 5 integers

  if (ptr == NULL) {
    printf("Memory allocation failed\n");
    return 1;
  }

  // Assign values to the dynamically allocated memory
  for (int i = 0; i < 5; i++) {
    ptr[i] = i * 10; // Use pointer like an array
  }

  // Print the values
  for (int i = 0; i < 5; i++) {
    printf("ptr[%d] = %d\n", i, ptr[i]);
  }

  free(ptr); // Free the allocated memory
  return 0;
}