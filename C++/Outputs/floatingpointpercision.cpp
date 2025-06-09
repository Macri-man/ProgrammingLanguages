#include <iostream>
#include <iomanip>

int main() {
    double value = 12345.6789;

    std::cout << "Default: " << value << std::endl;

    std::cout << std::fixed << "Fixed-point: " << value << std::endl;
    std::cout << std::scientific << "Scientific: " << value << std::endl;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Fixed-point with 2 decimals: " << value << std::endl;

    return 0;
}