#include <stdio.h>

typedef struct {
  char name[50];
  int age;
} Person;

void incrementAge(Person *p) { p->age++; }

int main() {
  Person person1 = {"Bob", 30};
  incrementAge(&person1);
  printf("Name: %s\n", person1.name);
  printf("Age: %d\n", person1.age);
  return 0;
}