#include <iostream>

int main() {
    int number = 255;

    std::cout << "Decimal: " << number << std::endl;
    std::cout << "Hexadecimal: " << std::hex << number << std::endl;
    std::cout << "Octal: " << std::oct << number << std::endl;

    return 0;
}