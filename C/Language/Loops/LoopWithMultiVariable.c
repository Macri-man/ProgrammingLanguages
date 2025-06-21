#include <stdio.h>

int main() {
  for (int fornumber1 = 0, fornumber2 = 5; fornumber1 < 5;
       fornumber1++, fornumber2--) {
    printf("i = %d, j = %d\n", fornumber1, fornumber2);
  }

  int whilenumber1 = 0, whilenumber2 = 5;

  while (whilenumber1 < 5 && whilenumber2 > 0) {
    printf("i = %d, j = %d\n", whilenumber1, whilenumber2);
    whilenumber1++; // increment i
    whilenumber2--; // decrement j
  }

  int dowhilenumber1 = 0, dowhilenumber2 = 5;

  do {
    printf("i = %d, j = %d\n", dowhilenumber1, dowhilenumber2);
    dowhilenumber1++; // increment i
    dowhilenumber2--; // decrement j
  } while (dowhilenumber1 < 5 && dowhilenumber2 > 0);

  return 0;
}