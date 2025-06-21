class MyClass {
public:
  void setValue(int val) { value = val; }

  int getValue() const { return value; }

private:
  int value;
};

// Outside the class definition
void MyClass::setValue(int val) { value = val; }