#include <stdio.h>

typedef struct {
  char name[50];
  int age;
} Person;

void printPerson(Person p) {
  printf("Name: %s\n", p.name);
  printf("Age: %d\n", p.age);
}

int main() {
  Person person1 = {"Alice", 25};
  printPerson(person1);
  return 0;
}