#include <stdio.h>

int main() {
  int number;
  printf("Enter a number: ");
  fscanf(stdin, "%d", &number); // Reads from standard input using fscanf
  printf("You entered: %d\n", number);
  return 0;
}