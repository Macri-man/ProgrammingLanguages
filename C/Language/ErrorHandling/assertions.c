#include <assert.h>
#include <stdio.h>


void divide(int a, int b) {
  assert(b != 0); // Assert that the divisor is not zero
  printf("Result: %d\n", a / b);
}

int main() {
  divide(10, 0); // This will cause the assertion to fail
  return 0;
}