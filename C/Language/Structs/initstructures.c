#include <stdio.h>

// Define a structure
typedef struct {
  char name[50];
  int age;
  float height;
} Person;

int main() {
  // Initialize a Person structure at declaration
  Person person1 = {"Alice", 25, 175.5};

  // Print the values
  printf("Name: %s\n", person1.name);
  printf("Age: %d\n", person1.age);
  printf("Height: %.2f cm\n", person1.height);

  return 0;
}