class MyClass {
public:
  // Constructor
  MyClass() { std::cout << "Constructor called." << std::endl; }

  // Destructor
  ~MyClass() { std::cout << "Destructor called." << std::endl; }

private:
  int value;
};