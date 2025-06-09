#include <iostream>

int main() {
  auto add = [](int a, int b) { return a + b; };

  std::cout << "Sum: " << add(5, 3) << std::endl;
  return 0;
}