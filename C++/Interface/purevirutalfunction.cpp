#include <iostream>

class IShape {
public:
  virtual void draw() const = 0; // Pure virtual function
  virtual ~IShape() = default;   // Virtual destructor
};

class Circle : public IShape {
public:
  void draw() const override { std::cout << "Drawing a circle." << std::endl; }
};

class Square : public IShape {
public:
  void draw() const override { std::cout << "Drawing a square." << std::endl; }
};

int main() {
  Circle circle;
  Square square;

  IShape *shapes[] = {&circle, &square};
  for (IShape *shape : shapes) {
    shape->draw(); // Calls the appropriate draw() implementation
  }

  return 0;
}