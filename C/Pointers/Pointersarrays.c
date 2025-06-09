#include <stdio.h>

int main() {
  int arr[] = {1, 2, 3, 4, 5};
  int *ptr = arr; // Points to the first element of the array

  for (int i = 0; i < 5; i++) {
    printf("arr[%d] = %d\n", i,
           *(ptr + i)); // Access array elements via pointer
  }

  return 0;
}