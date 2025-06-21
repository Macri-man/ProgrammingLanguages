#include <iostream>

enum class ErrorCode { SUCCESS, DIVIDE_BY_ZERO, UNKNOWN_ERROR };

ErrorCode divide(int a, int b, int &result) {
  if (b == 0) {
    return ErrorCode::DIVIDE_BY_ZERO;
  }
  result = a / b;
  return ErrorCode::SUCCESS;
}

int main() {
  int result;
  ErrorCode code = divide(10, 0, result);

  switch (code) {
  case ErrorCode::SUCCESS:
    std::cout << "Result: " << result << std::endl;
    break;
  case ErrorCode::DIVIDE_BY_ZERO:
    std::cerr << "Error: Division by zero" << std::endl;
    break;
  default:
    std::cerr << "Unknown error" << std::endl;
    break;
  }

  return 0;
}