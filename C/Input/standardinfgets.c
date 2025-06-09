#include <stdio.h>

int main() {
  char str[100];
  printf("Enter a string: ");
  fgets(str, sizeof(str),
        stdin); // Reads up to 99 characters from standard input
  printf("You entered: %s", str);
  return 0;
}