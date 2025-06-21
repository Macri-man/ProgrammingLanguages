#include <stdio.h>

// Define a structure
typedef struct {
  char name[50];
  int age;
  float height;
} Person;

int main() {
  // Declare a variable of type Person
  Person person1;

  // Initialize the members
  person1.age = 25;
  person1.height = 175.5;
  snprintf(person1.name, sizeof(person1.name), "Alice");

  // Print the values
  printf("Name: %s\n", person1.name);
  printf("Age: %d\n", person1.age);
  printf("Height: %.2f cm\n", person1.height);

  return 0;
}