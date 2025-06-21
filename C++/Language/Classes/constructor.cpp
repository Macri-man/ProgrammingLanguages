class MyClass {
public:
  // Default constructor
  MyClass() : value(0) {}

  // Parameterized constructor
  MyClass(int val) : value(val) {}

  // Copy constructor
  MyClass(const MyClass &other) : value(other.value) {}

  void display() const { std::cout << "Value: " << value << std::endl; }

private:
  int value;
};