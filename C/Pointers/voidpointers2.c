#include <stdio.h>

struct Person {
  int age;
  float height;
};

void printPerson(void *ptr) {
  // Cast the void pointer to Person pointer
  struct Person *personPtr = (struct Person *)ptr;

  // Access and print the structure members
  printf("Age: %d\n", personPtr->age);
  printf("Height: %.2f cm\n", personPtr->height);
}

int main() {

  void *ptr = (void *)5;
  int value = *(int *)ptr;
  printf("Num: %d\n", *(int *)ptr);

  struct Person person = {25, 175.5};
  void *ptr2 = (void *)&person;

  printPerson(ptr2);

  return 0;
}