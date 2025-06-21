#include <iostream>
#include <string>

int main() {
    int age;
    std::string fullName;

    std::cout << "Enter your age: ";
    std::cin >> age;

    std::cin.ignore(); // Clear the newline character left by 'cin'

    std::cout << "Enter your full name: ";
    std::getline(std::cin, fullName); // Now getline works correctly

    std::cout << "Your age is: " << age << " and your full name is: " << fullName << std::endl;

    return 0;
}