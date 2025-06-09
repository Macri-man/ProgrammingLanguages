#include <iostream>

class MyClass {
public:
  MyClass(int x, int y) : x(x), y(y) { // Constructor initialization list
    std::cout << "Constructor called with x: " << x << " and y: " << y
              << std::endl;
  }

private:
  int x;
  int y;
};

int main() {
  MyClass obj(5, 10); // Calls the constructor with initialization list
  return 0;
}