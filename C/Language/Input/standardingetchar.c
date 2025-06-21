#include <stdio.h>

int main() {
  char ch;
  printf("Enter a character: ");
  ch = getchar(); // Reads a single character
  printf("You entered: %c\n", ch);
  return 0;
}