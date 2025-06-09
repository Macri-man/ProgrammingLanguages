#include <iostream>

class MyClass {
public:
  MyClass(int value) { // Parameterized constructor
    this->value = value;
    std::cout << "Parameterized constructor called with value: " << value
              << std::endl;
  }

private:
  int value;
};

int main() {
  MyClass obj(10); // Calls the parameterized constructor with value 10
  return 0;
}