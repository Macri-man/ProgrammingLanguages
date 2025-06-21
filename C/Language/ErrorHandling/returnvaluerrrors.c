#include <stdio.h>

#define SUCCESS 0
#define ERROR_INVALID_INPUT 1
#define ERROR_OUT_OF_MEMORY 2

int process(int value) {
  if (value < 0) {
    return ERROR_INVALID_INPUT; // Return an error code
  }
  // Simulate some processing...
  return SUCCESS; // Return success code
}

int main() {
  int result = process(-1);

  if (result == ERROR_INVALID_INPUT) {
    fprintf(stderr,
            "Invalid input provided\n"); // Print error message to stderr
    return 1;
  }

  // Continue processing...

  return 0;
}