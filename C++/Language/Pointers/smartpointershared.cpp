#include <iostream>
#include <memory>


int main() {
  std::shared_ptr<int> ptr1 = std::make_shared<int>(20);
  std::shared_ptr<int> ptr2 = ptr1; // Both pointers share ownership

  std::cout << "Value: " << *ptr1 << std::endl; // Outputs: Value: 20
  std::cout << "Value: " << *ptr2 << std::endl; // Outputs: Value: 20

  // Memory is automatically managed; deallocated when last shared_ptr is
  // destroyed
  return 0;
}