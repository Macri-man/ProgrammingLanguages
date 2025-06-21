#include <iostream>
#include <utility> // For std::move

class MyClass {
public:
  // Parameterized constructor
  MyClass(int v) : value(new int(v)) {
    std::cout << "Parameterized constructor called, value: " << *value
              << std::endl;
  }

  // Move constructor
  MyClass(MyClass &&other) noexcept : value(other.value) {
    other.value = nullptr;
    std::cout << "Move constructor called." << std::endl;
  }

  // Destructor
  ~MyClass() { delete value; }

private:
  int *value;
};

int main() {
  MyClass obj1(42);
  MyClass obj2 = std::move(obj1); // Calls move constructor
  return 0;
}