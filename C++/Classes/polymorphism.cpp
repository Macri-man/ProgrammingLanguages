class Base {
public:
  virtual void show() const {
    std::cout << "Base class show function" << std::endl;
  }

  virtual ~Base() = default; // Virtual destructor
};

class Derived : public Base {
public:
  void show() const override {
    std::cout << "Derived class show function" << std::endl;
  }
};

int main() {
  Base *ptr = new Derived();
  ptr->show(); // Calls Derived's show function
  delete ptr;
  return 0;
}