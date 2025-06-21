#include <stdio.h>
#include <stdlib.h>

int main() {
  int *array = (int *)malloc(10 * sizeof(int));

  if (array == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

  // Use the array...
  free(array);

  return 0;
}