#include <stdio.h>

int main() {
    float num = 3.1415926535;

    // Specify field width (10 characters) and precision (2 decimal places)
    printf("%10.2f\n", num);  // Prints "      3.14" with padding

    // Print floating-point with 6 decimal places
    printf("%.6f\n", num);    // Prints "3.141593"

    return 0;
}