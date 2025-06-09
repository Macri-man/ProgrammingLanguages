#include <iostream>

void printMessage(
    std::string message = "Default Message"); // Function declaration

int main() {
  printMessage();                 // Uses default parameter
  printMessage("Custom Message"); // Uses provided parameter
  return 0;
}

void printMessage(std::string message) { // Function definition
  std::cout << message << std::endl;
}