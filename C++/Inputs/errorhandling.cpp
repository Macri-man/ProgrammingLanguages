#include <iostream>

int main() {
    int number;

    std::cout << "Enter an integer: ";
    std::cin >> number;

    if (std::cin.fail()) {
        std::cout << "Invalid input!" << std::endl;
        std::cin.clear(); // Clear the error state
        std::cin.ignore(1000, '\n'); // Ignore remaining characters in the buffer
    } else {
        std::cout << "You entered: " << number << std::endl;
    }

    return 0;
}