#include <stdio.h>

// Define a structure
typedef struct {
  char street[100];
  char city[50];
} Address;

typedef struct {
  char name[50];
  int age;
  Address address; // Nested structure
} Person;

int main() {
  // Initialize a Person structure with nested Address
  Person person1 = {"John Doe", 28, {"123 Elm Street", "Springfield"}};

  // Print the values
  printf("Name: %s\n", person1.name);
  printf("Age: %d\n", person1.age);
  printf("Address: %s, %s\n", person1.address.street, person1.address.city);

  return 0;
}