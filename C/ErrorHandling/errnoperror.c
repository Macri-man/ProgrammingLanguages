#include <errno.h>
#include <stdio.h>


int main() {
  FILE *file = fopen("nonexistent_file.txt", "r");

  if (file == NULL) {
    perror("File open error");         // Print a descriptive error message
    printf("Error code: %d\n", errno); // Print the error code
    return 1;
  }

  fclose(file);
  return 0;
}