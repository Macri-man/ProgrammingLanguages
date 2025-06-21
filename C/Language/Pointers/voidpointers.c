#include <stdio.h>

// Function to add two integers using void pointers
void addIntegers(void *a, void *b, void *result) {
  // Cast the void pointers to integer pointers
  *(int *)result = *(int *)a + *(int *)b;
}

// Function to add two floats using void pointers
void addFloats(void *a, void *b, void *result) {
  // Cast the void pointers to float pointers
  *(float *)result = *(float *)a + *(float *)b;
}

// Function to add two doubles using void pointers
void addDoubles(void *a, void *b, void *result) {
  // Cast the void pointers to double pointers
  *(double *)result = *(double *)a + *(double *)b;
}

int main() {
  // Variables for integers, floats, and doubles
  int int1 = 10, int2 = 20, intResult;
  float float1 = 5.5, float2 = 10.5, floatResult;
  double double1 = 2.5, double2 = 3.5, doubleResult;

  // Add integers
  addIntegers(&int1, &int2, &intResult);
  printf("Sum of integers: %d\n", intResult);

  // Add floats
  addFloats(&float1, &float2, &floatResult);
  printf("Sum of floats: %.2f\n", floatResult);

  // Add doubles
  addDoubles(&double1, &double2, &doubleResult);
  printf("Sum of doubles: %.2lf\n", doubleResult);

  return 0;
}