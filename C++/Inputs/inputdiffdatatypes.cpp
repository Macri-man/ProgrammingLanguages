#include <iostream>

int main() {
    int age;
    double height;
    char gender;
    std::string name;

    std::cout << "Enter your name: ";
    std::cin >> name;  // Input string
    std::cout << "Enter your age: ";
    std::cin >> age;   // Input integer
    std::cout << "Enter your height in meters: ";
    std::cin >> height; // Input double
    std::cout << "Enter your gender (M/F): ";
    std::cin >> gender; // Input char

    std::cout << "Name: " << name << ", Age: " << age << ", Height: " << height << "m, Gender: " << gender << std::endl;

    return 0;
}