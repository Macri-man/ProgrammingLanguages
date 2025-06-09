#include <iostream>

int main() {
  int *arr = new int[5]; // Allocate memory for an array of 5 integers

  for (int i = 0; i < 5; ++i) {
    arr[i] = i * 10;
  }

  for (int i = 0; i < 5; ++i) {
    std::cout << arr[i] << " "; // Outputs: 0 10 20 30 40
  }

  delete[] arr; // Deallocate the memory for the array
  return 0
}