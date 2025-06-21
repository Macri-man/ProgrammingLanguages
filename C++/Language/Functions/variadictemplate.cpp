#include <iostream>

// Base case: No arguments
void print() { std::cout << std::endl; }

// Recursive case: At least one argument
template <typename T, typename... Args> void print(T first, Args... rest) {
  std::cout << first << " ";
  print(rest...); // Recursive call with the rest of the arguments
}

int main() {
  print(1, 2, 3);           // Prints: 1 2 3
  print("Hello", 42, 3.14); // Prints: Hello 42 3.14
  return 0;
}