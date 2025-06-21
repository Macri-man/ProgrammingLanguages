#include <stdio.h>

int main() {
    char buffer[50];
    int num = 123;
    
    // Store formatted string in buffer
    sprintf(buffer, "Number: %d", num);
    
    // Print the formatted string from the buffer
    printf("%s\n", buffer);

    return 0;
}