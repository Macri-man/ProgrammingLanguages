#include <exception> // For std::exception
#include <iostream>


void mayThrow() noexcept {
  // Function logic here
  // This function is guaranteed not to throw any exceptions
  std::cout << "Function executed successfully without throwing exceptions."
            << std::endl;
}

int main() {
  try {
    mayThrow(); // Call the function
  } catch (const std::exception &e) {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
  }

  return 0;
}