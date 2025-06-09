#include <iostream>

int main() {
    std::cout << "Processing..." << std::flush; // Print without a newline, but flush the buffer
    // Simulate a delay (in reality, some processing would be happening here)
    for (int i = 0; i < 100000000; ++i);
    std::cout << " Done!" << std::endl;
    return 0;
}