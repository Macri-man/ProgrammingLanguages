#include <iostream>
#include <string>

int main() {
    std::string fullName;

    std::cout << "Enter your full name: ";
    std::getline(std::cin, fullName); // Reads a full line including spaces
    std::cout << "Your full name is: " << fullName << std::endl;

    return 0;
}