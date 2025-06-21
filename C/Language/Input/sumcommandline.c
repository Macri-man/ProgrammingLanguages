#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please provide numbers to sum.\n");
    return 1;
  }

  int sum = 0;

  // Start from 1, since argv[0] is the program name
  for (int i = 1; i < argc; i++) {
    sum += atoi(argv[i]); // Convert the argument to an integer
  }

  printf("Sum: %d\n", sum);

  return 0;
}