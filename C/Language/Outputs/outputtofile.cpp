FILE *file = fopen("filename", "mode"); opens a file.
fprintf() is used to write formatted data to a file.
fclose() closes the file.

#include <stdio.h>

int main() {
    FILE *file = fopen("output.txt", "w");
    
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    int num = 100;
    fprintf(file, "Writing number: %d\n", num);
    
    fclose(file);
    
    printf("Data written to file successfully.\n");

    return 0;
}