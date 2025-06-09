#include <iostream>

int main() {
  int matrix[3][3] = {// 2D array initialization
                      {1, 2, 3},
                      {4, 5, 6},
                      {7, 8, 9}};

  // Accessing elements
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      std::cout << "matrix[" << i << "][" << j << "] = " << matrix[i][j]
                << std::endl;
    }
  }
  return 0;
}