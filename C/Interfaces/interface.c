#include <stdio.h>

// Shape interface definition using function pointers
struct Shape {
  double (*area)(void *self);
  double (*perimeter)(void *self);
};

struct Circle {
  struct Shape shape; // Inherits the "interface"
  double radius;
};

// Circle's area function
double circle_area(void *self) {
  struct Circle *circle = (struct Circle *)self;
  return 3.14159 * circle->radius * circle->radius;
}

// Circle's perimeter function
double circle_perimeter(void *self) {
  struct Circle *circle = (struct Circle *)self;
  return 2 * 3.14159 * circle->radius;
}

struct Rectangle {
  struct Shape shape; // Inherits the "interface"
  double width, height;
};

// Rectangle's area function
double rectangle_area(void *self) {
  struct Rectangle *rect = (struct Rectangle *)self;
  return rect->width * rect->height;
}

// Rectangle's perimeter function
double rectangle_perimeter(void *self) {
  struct Rectangle *rect = (struct Rectangle *)self;
  return 2 * (rect->width + rect->height);
}

int main() {
  // Circle
  struct Circle circle = {{circle_area, circle_perimeter}, 5};

  // Rectangle
  struct Rectangle rect = {{rectangle_area, rectangle_perimeter}, 4, 6};

  // Accessing circle's functions via the "interface"
  printf("Circle area: %f\n", circle.shape.area(&circle));
  printf("Circle perimeter: %f\n", circle.shape.perimeter(&circle));

  // Accessing rectangle's functions via the "interface"
  printf("Rectangle area: %f\n", rect.shape.area(&rect));
  printf("Rectangle perimeter: %f\n", rect.shape.perimeter(&rect));

  return 0;
}