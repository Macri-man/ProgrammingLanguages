#include <iostream>

int main() {
  int arr[3] = {1, 2, 3};
  int *ptr = arr;

  std::cout << "First element: " << *ptr
            << std::endl; // Outputs: First element: 1
  ptr++;
  std::cout << "Second element: " << *ptr
            << std::endl; // Outputs: Second element: 2
  return 0
}