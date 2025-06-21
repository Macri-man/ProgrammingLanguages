#include <stdarg.h>
#include <stdio.h>


// Function that accepts a variable number of arguments
int sum(int count, ...) {
  va_list args;
  va_start(args, count);

  int total = 0;
  for (int i = 0; i < count; i++) {
    total += va_arg(args, int); // Get the next argument
  }

  va_end(args);
  return total;
}

int main() {
  printf("Sum of 2, 3, 4: %d\n", sum(3, 2, 3, 4)); // Output: 9
  return 0;
}