class MyClass {
public:
  static int staticValue; // Static member variable

  static void staticFunction() { // Static member function
    std::cout << "Static function called, value: " << staticValue << std::endl;
  }
};

int MyClass::staticValue = 5; // Initialize static member

int main() {
  MyClass::staticFunction(); // Call static member function
  return 0;
}