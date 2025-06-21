#include <iostream>
#include <iomanip> // Required for manipulators

int main() {
    double pi = 3.14159;

    // Print a number with a fixed number of decimal places
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Pi rounded to two decimal places: " << pi << std::endl;

    // Print multiple values with width formatting
    std::cout << std::setw(10) << 123 << std::setw(10) << 456 << std::endl;

    return 0;
}