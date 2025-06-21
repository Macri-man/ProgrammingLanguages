#include <stdio.h>

int main() {
  int age;
  char name[50];

  // Read integer input
  printf("Enter your age: ");
  scanf("%d", &age);

  // Clear the input buffer to handle next input correctly
  getchar(); // This consumes the leftover newline character

  // Read string input using fgets()
  printf("Enter your name: ");
  fgets(name, sizeof(name), stdin);

  // Display the input
  printf("Your name is %s and you are %d years old.\n", name, age);

  return 0;
}