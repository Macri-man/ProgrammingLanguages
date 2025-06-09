#include <stdio.h>

int main() {
    int x = 42;
    int *ptr = &x;

    // Print the address of x
    printf("Address of x: %p\n", (void *)ptr);

    return 0;
}