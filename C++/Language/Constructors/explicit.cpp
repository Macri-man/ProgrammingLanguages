#include <iostream>

class MyClass {
public:
  explicit MyClass(int value) : value(value) {
    std::cout << "Explicit constructor called with value: " << value
              << std::endl;
  }

private:
  int value;
};

int main() {
  MyClass obj1(10); // OK
  // MyClass obj2 = 20; // Error: implicit conversion not allowed with explicit
  // constructor
  return 0;
}