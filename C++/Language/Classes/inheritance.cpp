class Base {
public:
  void baseFunction() { std::cout << "Function in Base class" << std::endl; }
};

class Derived : public Base {
public:
  void derivedFunction() {
    std::cout << "Function in Derived class" << std::endl;
  }
};

int main() {
  Derived obj;
  obj.baseFunction();    // Calls function from Base class
  obj.derivedFunction(); // Calls function from Derived class
  return 0;
}