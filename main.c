#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 256

// Define the structure for DomesticStudent and InternationalStudent
typedef struct {
    char firstName[50];
    char lastName[50];
    int year;
    int month;
    int day;
    float gpa;
    char status;  // 'D' for domestic
} DomesticStudent;

typedef struct {
    char firstName[50];
    char lastName[50];
    int year;
    int month;
    int day;
    float gpa;
    char status;  // 'I' for international
    int toefl;
} InternationalStudent;

// Function prototypes
void processFile(FILE *input, FILE *output, int option);

// Helper function to check if a string is a valid float
bool isFloat(const char *str) {
    char *endptr;
    strtod(str, &endptr);
    return *endptr == '\0';
}

// Helper function to check if a string is a valid integer
bool isInteger(const char *str) {
    char *endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

// Function to check if the line has at least four arguments (minimum for 'D' status)
bool isValidLine(const char *line) {
    int spaceCount = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == ' ') {
            spaceCount++;
        }
    }
    return spaceCount >= 3;
}

// Function to validate line format with specific error reporting
bool validateLineFormat(const char *line, bool *requiresTOEFL) {
    char firstName[50], lastName[50], gpaStr[20], statusChar, toeflStr[20];
    int fieldsRead;

    // First, check if the line has at least four space-separated arguments
    if (!isValidLine(line)) {
        fprintf(stderr, "Error: Invalid format - Line must contain at least FirstName, LastName, GPA, and Status\n");
        return false;
    }

    // Attempt to parse the line into components
    fieldsRead = sscanf(line, "%49s %49s %19s %c %19s", firstName, lastName, gpaStr, &statusChar, toeflStr);

    // Check if the name fields are alphabetic
    for (int i = 0; i < strlen(firstName); i++) {
        if (!isalpha(firstName[i])) {
            fprintf(stderr, "Error: Invalid first name - Contains non-alphabetical characters: %s\n", firstName);
            return false;
        }
    }
    for (int i = 0; i < strlen(lastName); i++) {
        if (!isalpha(lastName[i])) {
            fprintf(stderr, "Error: Invalid last name - Contains non-alphabetical characters: %s\n", lastName);
            return false;
        }
    }

    // Check if GPA is a valid float
    if (!isFloat(gpaStr)) {
        fprintf(stderr, "Error: Invalid GPA - Not a valid floating-point number: %s\n", gpaStr);
        return false;
    }

    // Check if status is either 'I' or 'D'
    if (statusChar != 'I' && statusChar != 'D') {
        fprintf(stderr, "Error: Invalid status - Expected 'I' or 'D', found: %c\n", statusChar);
        return false;
    }

    // If status is 'I', ensure TOEFL score is provided and valid
    if (statusChar == 'I') {
        *requiresTOEFL = true;
        if (fieldsRead != 5 || !isInteger(toeflStr)) {
            fprintf(stderr, "Error: Invalid TOEFL score - Expected an integer: %s\n", toeflStr);
            return false;
        }
    } else {
        *requiresTOEFL = false;
        if (fieldsRead > 4) {
            fprintf(stderr, "Error: Extra arguments found for 'D' status\n");
            return false;
        }
    }

    return true;
}

// Function to compare two students based on the sorting criteria
int compareStudents(const void *a, const void *b) {
    DomesticStudent *studentA = (DomesticStudent *)a;
    DomesticStudent *studentB = (DomesticStudent *)b;

    // Compare by year of birth
    if (studentA->year != studentB->year) {
        return studentA->year - studentB->year;
    }

    // Compare by month of birth
    if (studentA->month != studentB->month) {
        return studentA->month - studentB->month;
    }

    // Compare by day of birth
    if (studentA->day != studentB->day) {
        return studentA->day - studentB->day;
    }

    // Compare by last name
    int lastNameComparison = strcmp(studentA->lastName, studentB->lastName);
    if (lastNameComparison != 0) {
        return lastNameComparison;
    }

    // Compare by first name
    int firstNameComparison = strcmp(studentA->firstName, studentB->firstName);
    if (firstNameComparison != 0) {
        return firstNameComparison;
    }

    // Compare by GPA
    if (studentA->gpa != studentB->gpa) {
        return (studentA->gpa > studentB->gpa) ? -1 : 1; // Higher GPA comes first
    }

    // Compare by TOEFL if available
    if (studentA->status == 'I' && studentB->status == 'I') {
        InternationalStudent *intlA = (InternationalStudent *)studentA;
        InternationalStudent *intlB = (InternationalStudent *)studentB;
        return intlB->toefl - intlA->toefl;
    }

    // Domestic students take precedence over international
    if (studentA->status != studentB->status) {
        return (studentA->status == 'D') ? -1 : 1;
    }

    return 0;
}

// Merge function for MergeSort
void merge(void *arr, int left, int mid, int right, size_t size, int (*cmp)(const void *, const void *)) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    void *L = malloc(n1 * size);
    void *R = malloc(n2 * size);

    for (int i = 0; i < n1; i++) {
        memcpy((char *)L + i * size, (char *)arr + (left + i) * size, size);
    }
    for (int j = 0; j < n2; j++) {
        memcpy((char *)R + j * size, (char *)arr + (mid + 1 + j) * size, size);
    }

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (cmp((char *)L + i * size, (char *)R + j * size) <= 0) {
            memcpy((char *)arr + k * size, (char *)L + i * size, size);
            i++;
        } else {
            memcpy((char *)arr + k * size, (char *)R + j * size, size);
            j++;
        }
        k++;
    }

    while (i < n1) {
        memcpy((char *)arr + k * size, (char *)L + i * size, size);
        i++;
        k++;
    }

    while (j < n2) {
        memcpy((char *)arr + k * size, (char *)R + j * size, size);
        j++;
        k++;
    }

    free(L);
    free(R);
}

// MergeSort function
void mergeSort(void *arr, int left, int right, size_t size, int (*cmp)(const void *, const void *)) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid, size, cmp);
        mergeSort(arr, mid + 1, right, size, cmp);
        merge(arr, left, mid, right, size, cmp);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Error: Insufficient arguments\n");
        return 1;
    }

    // Open input and output files
    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        printf("Error: Could not open input file\n");
        return 1;
    }

    FILE *outputFile = fopen(argv[2], "w");
    if (outputFile == NULL) {
        printf("Error: Could not open output file\n");
        fclose(inputFile);
        return 1;
    }

    // Get option from command line argument
    int option = atoi(argv[3]);
    if (option < 1 || option > 3) {
        fprintf(outputFile, "Error: Invalid option\n");
        fclose(inputFile);
        fclose(outputFile);
        return 1;
    }

    // Process the file based on the given option
    processFile(inputFile, outputFile, option);

    fclose(inputFile);
    fclose(outputFile);

    return 0;
}

void processFile(FILE *input, FILE *output, int option) {
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), input)) {
        // Validate the format of the line
        if (!isValidLine(line)) {
            fprintf(output, "Error: Invalid format\n");
            return;
        }

        char firstName[50], lastName[50], status;
        float gpa;
        int toefl = -1;
        bool requiresTOEFL = false;

        // Validate the line format
        if (!validateLineFormat(line, &requiresTOEFL)) {
            // If validation fails, stop processing this line
            return;
        }

        // Parse the line based on the student's status
        if (requiresTOEFL) {
            sscanf(line, "%49s %49s %f %c %d", firstName, lastName, &gpa, &status, &toefl);
            fprintf(output, "First Name: %s, Last Name: %s, GPA: %.3f, Status: %c, TOEFL: %d\n", firstName, lastName, gpa, status, toefl);
        } else {
            sscanf(line, "%49s %49s %f %c", firstName, lastName, &gpa, &status);
            fprintf(output, "First Name: %s, Last Name: %s, GPA: %.3f, Status: %c\n", firstName, lastName, gpa, status);
        }

        // Check GPA and status
        if (gpa > 3.9) {
            if (status == 'D' && option != 2) {
                DomesticStudent ds = {0};
                strcpy(ds.firstName, firstName);
                strcpy(ds.lastName, lastName);
                ds.gpa = gpa;
                ds.status = status;

                fprintf(output, "%s %s %.3f %c\n", ds.firstName, ds.lastName, ds.gpa, ds.status);
            } else if (status == 'I' && option != 1 && toefl >= 70) {
                InternationalStudent is = {0};
                strcpy(is.firstName, firstName);
                strcpy(is.lastName, lastName);
                is.gpa = gpa;
                is.status = status;
                is.toefl = toefl;

                fprintf(output, "%s %s %.3f %c %d\n", is.firstName, is.lastName, is.gpa, is.status, is.toefl);
            }
        }
    }
}

