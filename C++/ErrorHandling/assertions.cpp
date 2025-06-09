#include <cassert>

int main() {
  int x = 10;
  assert(x == 10); // No error
  assert(x == 5);  // Will terminate the program
  return 0;
}