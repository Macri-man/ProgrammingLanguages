#include <iostream>

void printArray(int arr[], int size) { // Function to print array elements
  for (int i = 0; i < size; ++i) {
    std::cout << arr[i] << " ";
  }
  std::cout << std::endl;
}

int main() {
  int numbers[5] = {1, 2, 3, 4, 5};
  printArray(numbers, 5); // Passing array to function
  return 0;
}