#include <stdio.h>

int main() {
  FILE *file = fopen("nonexistent_file.txt", "r");

  if (file == NULL) {
    perror("Error opening file"); // Print a descriptive error message
    return 1; // Return a non-zero value to indicate an error
  }

  // Use the file...
  fclose(file);

  return 0; // Return 0 to indicate success
}