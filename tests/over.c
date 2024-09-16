#include <stdio.h>
#include <string.h>

int main() {
    char buffer[10];    // Small buffer
    int secret_var = 42;  // Variable to be potentially overwritten

    printf("Original value of secret_var: %d\n", secret_var);

    // Prompt user for input
    printf("Enter a string: ");
    scanf("%s", buffer);  // No limit on input size

    // Print buffer content and the value of secret_var
    printf("Buffer content: %s\n", buffer);
    printf("Value of secret_var: %d\n", secret_var);

    return 0;
}
