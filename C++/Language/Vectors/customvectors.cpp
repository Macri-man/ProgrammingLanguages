#include <iostream>
#include <stdexcept>

template <typename T> class Vector {
private:
  T *arr;
  size_t capacity;
  size_t current;

public:
  Vector() {
    arr = new T[1];
    capacity = 1;
    current = 0;
  }

  ~Vector() { delete[] arr; }

  void push_back(T data) {
    if (current == capacity) {
      T *temp = new T[2 * capacity];
      for (size_t i = 0; i < capacity; i++) {
        temp[i] = arr[i];
      }
      delete[] arr;
      capacity *= 2;
      arr = temp;
    }
    arr[current] = data;
    current++;
  }

  void pop_back() {
    if (current > 0) {
      current--;
    }
  }

  T &operator[](size_t index) {
    if (index >= current) {
      throw std::out_of_range("Index out of range");
    }
    return arr[index];
  }

  size_t size() const { return current; }

  size_t get_capacity() const { return capacity; }
};

// Example usage
int main() {
  Vector<int> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  std::cout << "Vector size: " << v.size() << std::endl;
  std::cout << "Vector capacity: " << v.get_capacity() << std::endl;

  std::cout << "Elements: ";
  for (size_t i = 0; i < v.size(); i++) {
    std::cout << v[i] << " ";
  }
  std::cout << std::endl;

  return 0;
}