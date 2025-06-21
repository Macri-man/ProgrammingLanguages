#include <iostream>

int main() {
  void display(int num) { std::cout << "Number: " << num << std::endl; }

  void (*func_ptr)(int) = display;

  func_ptr(5); // Calls the display function through the pointer
  return 0
}