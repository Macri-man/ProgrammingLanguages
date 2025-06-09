/*
char	1 byte	-128 to 127 (or 0 to 255 for unsigned)
int	2 or 4 bytes	-32,768 to 32,767 (or larger on 4 bytes)
unsigned int	2 or 4 bytes	0 to 65,535 (on 2 bytes)
short	2 bytes	-32,768 to 32,767
unsigned short	2 bytes	0 to 65,535
long	4 or 8 bytes	-2,147,483,648 to 2,147,483,647
unsigned long	4 or 8 bytes	0 to 4,294,967,295
float	4 bytes	3.4E-38 to 3.4E+38
double	8 bytes	1.7E-308 to 1.7E+308
long double	10 or 16 bytes	3.4E-4932 to 1.1E+4932

int arr[5];  // Array of 5 integers
int *ptr;  // Pointer to an integer

struct Person {
    int age;
    float height;
};

union Data {
    int i;
    float f;
    char str[20];
};

enum Weekday { Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday };

void functionName();  // Function with no return type
void *ptr;            // Void pointer


Integer Types: int, unsigned int, short, long, unsigned long.
Floating-Point Types: float, double, long double.
Character Types: char.
Void Type: void.
Derived Types: Arrays, pointers, structures, unions.
Enumerations: Defined using enum.
*/

#include <stdio.h>

int main() {
    char letter = 'A';        // char data type
    int number = 100;         // int data type
    float pi = 3.14159;       // float data type
    double largeNum = 123456789.123456; // double data type

    printf("Character: %c\n", letter);  // Output a char
    printf("Integer: %d\n", number);    // Output an int
    printf("Float: %.2f\n", pi);        // Output a float with 2 decimal places
    printf("Double: %.6f\n", largeNum); // Output a double with 6 decimal places

    return 0;
}