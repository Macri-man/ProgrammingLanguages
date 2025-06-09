#include <iostream>

class MyClass {
public:
  MyClass() = default; // Default constructor explicitly defined

  MyClass(int value) = delete; // Delete constructor to prevent its use

  void show() { std::cout << "Default constructor used." << std::endl; }
};

int main() {
  MyClass obj; // OK
  // MyClass obj2(10); // Error: constructor is deleted
  obj.show();
  return 0;
}