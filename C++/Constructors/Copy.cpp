#include <iostream>

class MyClass {
public:
  MyClass(int value) : value(value) {} // Parameterized constructor

  MyClass(const MyClass &other) { // Copy constructor
    value = other.value;
    std::cout << "Copy constructor called, value: " << value << std::endl;
  }

private:
  int value;
};

int main() {
  MyClass obj1(10);    // Calls parameterized constructor
  MyClass obj2 = obj1; // Calls copy constructor
  return 0;
}