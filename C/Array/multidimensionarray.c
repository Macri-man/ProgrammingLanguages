#include <stdio.h>

int main() {
  // Declare and initialize a 2D array (3x3)
  int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

  // Access and print 2D array elements
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      printf("matrix[%d][%d] = %d\n", i, j, matrix[i][j]);
    }
  }

  return 0;
}