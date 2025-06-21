#include <iostream>

class MyClass {
public:
  // Constructor
  MyClass(int val) : value(val) {}

  // Member function
  void display() const { std::cout << "Value: " << value << std::endl; }

private:
  // Member variable
  int value;
};

int main() {
  MyClass obj(10); // Create an object of MyClass
  obj.display();   // Call the member function
  return 0;
}