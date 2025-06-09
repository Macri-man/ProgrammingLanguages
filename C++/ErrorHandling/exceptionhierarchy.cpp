#include <iostream>
#include <stdexcept>

class CustomException : public std::exception {
public:
  const char *what() const noexcept override {
    return "Custom exception occurred";
  }
};

int main() {
  try {
    throw CustomException();
  } catch (const CustomException &e) {
    std::cerr << "Custom exception caught: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Standard exception caught: " << e.what() << std::endl;
  }
  return 0;
}