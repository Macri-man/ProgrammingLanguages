#include <stdio.h>

int main() {

  for (int fornumber = 0; fornumber < 5; fornumber++) {
    if (fornumber == 3) {
      break; // exits the loop when i equals 3
    }
    printf("Iteration %d\n", fornumber);
  }
  int dowhilenumber = 0;
  do {
    if (dowhilenumber == 3) {
      break; // exits the loop when i equals 3
    }
    dowhilenumber++;
    printf("Iteration %d\n", dowhilenumber);
  } while (dowhilenumber < 6);

  int whilenumber = 0;
  while (whilenumber < 5) {
    if (whilenumber == 3) {
      break; // exits the loop when i equals 3
    }
    printf("Iteration %d\n", whilenumber);
    whilenumber++;
  }

  return 0;
}