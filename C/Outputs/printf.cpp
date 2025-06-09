#include <stdio.h>

int main() {
    int num = 10;
    float pi = 3.14;
    char grade = 'A';
    char name[] = "John";

    // Print an integer
    printf("Number: %d\n", num);
    
    // Print a floating-point number
    printf("Value of Pi: %.2f\n", pi);
    
    // Print a character
    printf("Grade: %c\n", grade);
    
    // Print a string
    printf("Name: %s\n", name);

    printf("Hello, World!\n");    // Prints with a newline
    printf("Column1\tColumn2\n"); // Prints with a tab
    printf("This is a backslash: \\\n"); // Prints a backslash

    return 0;
}


%d or %i: Integer.
%f: Floating-point.
%.nf: Floating-point with n decimal places (e.g., %.2f for two decimal places).
%c: Character.
%s: String.
%p: Pointer (memory address).
%u: Unsigned integer.
%x or %X: Hexadecimal representation of an integer.
%o: Octal representation of an integer.

Escape sequences allow you to include special characters like newlines, tabs, and backslashes in your output.

\n: Newline.
\t: Tab.
\\: Backslash.
\": Double quote.
