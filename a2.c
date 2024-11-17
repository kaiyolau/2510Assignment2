#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 256
#define MAX_STUDENTS 1000

// Define the structure for DomesticStudent and InternationalStudent
typedef struct {
    char birthDigits[50];
    char firstName[50];
    char lastName[50];
    int year, month, day;
    float gpa;
    char status;  // 'D' for domestic
} DomesticStudent;

typedef struct {
    char birthDigits[50];
    char firstName[50];
    char lastName[50];
    int year, month, day;
    float gpa;
    char status;  // 'I' for international
    int toefl;
} InternationalStudent;

// Function prototypes
void processFile(FILE *input, FILE *output, int option);
int getMonthNumber(const char *month);
void divideBirthDigits(const char *birthDigits, int *day, int *month, int *year);
int getMonthNumber(const char *month_of_birth);
void divideBirthDigits(const char *birthDigits, int *day, int *month, int *year);

const char* getMonthAbbreviation(int month) {
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    if (month >= 1 && month <= 12) {
        return months[month - 1];
    }
    return "Error: Invalid month";
}

// Jan -> 1, Feb -> 2, etc
int getMonthNumber(const char *month) {
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
        if (strcmp(month, months[i]) == 0) return i + 1;
    }
    return -1;
}

// Function to divide birthdate string into day, month, and year
void divideBirthDigits(const char *birthDigits, int *day, int *month, int *year) {
    char monthStr[4];
    sscanf(birthDigits, "%3s-%d-%d", monthStr, day, year);
    *month = getMonthNumber(monthStr);
}

// Helper function to check if a string is a valid float
int isFloat(const char *str) {
    char *endptr;
    strtod(str, &endptr);
    return *endptr == '\0';
}

// Helper function to check if a string is a valid integer
int isInteger(const char *str) {
    char *endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

// Function to check if the line has at least four arguments (minimum for 'D' status)
int isValidLine(const char *line) {
    int spaceCount = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == ' ') {
            spaceCount++;
        }
    }
    return spaceCount >= 3;
}

int isAlphabet(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Function to validate line format with specific error reporting
int validateLineFormat(const char *line, int *requiresTOEFL) {
    char birthDigits[50], firstName[50], lastName[50], gpaStr[20], statusChar, toeflStr[20];
    int fieldsRead;

    // Attempt to parse the line into components
    fieldsRead = sscanf(line, "%49s %49s %s %19s %c %19s", firstName, lastName, birthDigits, gpaStr, &statusChar, toeflStr);

    // Check if we have at least 4 fields
    if (fieldsRead < 5) {
        fprintf(stderr, "Error: Line must contain at least 5 fields (FirstName LastName BirthDate GPA Status)\n");
        return 0;
    }

    // Check if the name fields are alphabetic
    for (int i = 0; i < strlen(firstName); i++) {
        if (!isAlphabet(firstName[i])) {
            fprintf(stderr, "Error: Invalid first name - Contains non-alphabetical characters: %s\n", firstName);
            return 0;
        }
    }
    for (int i = 0; i < strlen(lastName); i++) {
        if (!isAlphabet(lastName[i])) {
            fprintf(stderr, "Error: Invalid last name - Contains non-alphabetical characters: %s\n", lastName);
            return 0;
        }
    }

    // Check if GPA is a valid float and within the range 0.0 to 4.3
    float gpa = atof(gpaStr);
    if (!isFloat(gpaStr) || gpa < 0.0 || gpa > 4.3) {
        fprintf(stderr, "Error: Invalid GPA - Not in range 0.0 to 4.3: %s\n", gpaStr);
        return 0;
    }

    // Check if status is either 'I' or 'D'
    if (statusChar != 'I' && statusChar != 'D') {
        fprintf(stderr, "Error: Invalid status - Expected 'I' or 'D', found: %c\n", statusChar);
        return 0;
    }

    // If status is 'I', ensure TOEFL score is provided and valid
    if (statusChar == 'I') {
        *requiresTOEFL = 1;
        if (fieldsRead != 6 || !isInteger(toeflStr)) {
            fprintf(stderr, "Error: Invalid TOEFL score - Expected an integer: %s\n", toeflStr);
            return 0;
        }
    } else {
        *requiresTOEFL = 0;
        if (fieldsRead > 5) {
            fprintf(stderr, "Error: Extra arguments found for 'D' status\n");
            return 0;
        }
    }

    return 1;
}

// Function to compare two students based on the sorting criteria
int compareStudents(const void *a, const void *b) {
    DomesticStudent *studentA = (DomesticStudent *)a;
    DomesticStudent *studentB = (DomesticStudent *)b;

    int dayA, monthA, yearA, dayB, monthB, yearB;

    // Divide birth digits into day, month, and year for both students
    divideBirthDigits(studentA->birthDigits, &dayA, &monthA, &yearA);
    divideBirthDigits(studentB->birthDigits, &dayB, &monthB, &yearB);

    // Compare by year, month, day of birth
    if (yearA != yearB) return yearA - yearB;
    if (monthA != monthB) return monthA - monthB;
    if (dayA != dayB) return dayA - dayB;

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

void processFile(FILE *input, FILE *output, int option) {
    char line[MAX_LINE_LENGTH];

    // Arrays to store students
    DomesticStudent domesticList[MAX_STUDENTS];
    InternationalStudent internationalList[MAX_STUDENTS];
    int domesticCount = 0, internationalCount = 0;

    while (fgets(line, sizeof(line), input)) {
        // Remove trailing newline character
        line[strcspn(line, "\n")] = '\0';

        char firstName[50], lastName[50], birthDigits[50];
        float gpa = 0.0;
        char status;
        int toefl = -1;
        int requiresTOEFL = 0;

        //Validate the format of the line
        if (!validateLineFormat(line, &requiresTOEFL)) {
            // If validation fails, stop processing this line
            fprintf(output, "Error: Invalid format\n");
            continue;
        }

        // Parse the line based on whether TOEFL is required or not
        int fieldsRead;
        if (requiresTOEFL) {
            fieldsRead = sscanf(line, "%49s %49s %49s %f %c %d", firstName, lastName, birthDigits, &gpa, &status, &toefl);
        } else {
            fieldsRead = sscanf(line, "%49s %49s %49s %f %c", firstName, lastName, birthDigits, &gpa, &status);
        }

        // Extract day, month, and year from birthDigits
        int day = 0, month = 0, year = 0;
        divideBirthDigits(birthDigits, &day, &month, &year);

        // Ensure valid month and year ranges
        if (month == -1 || year < 1950 || year > 2010) {
            fprintf(output, "Error: Invalid birth date\n");
            continue;
        }

        // Check if the status is 'I' or 'D'
        if (status != 'I' && status != 'D') {
            fprintf(output, "Error: Invalid status - Expected 'I' or 'D'\n");
            continue;
        }

        // Format the birth date once here
        char formattedBirthDigits[50];
        snprintf(formattedBirthDigits, sizeof(formattedBirthDigits), "%s-%d-%d", getMonthAbbreviation(month), day, year);

        // Handle domestic students
        if (status == 'D') {
            if (fieldsRead > 5) {
                fprintf(output, "Error: Extra arguments found for domestic student\n");
                continue;
            }
            DomesticStudent ds;
            strcpy(ds.firstName, firstName);
            strcpy(ds.lastName, lastName);
            strcpy(ds.birthDigits, formattedBirthDigits);
            ds.day = day;
            ds.month = month;
            ds.year = year;
            ds.gpa = gpa;
            ds.status = status;
            domesticList[domesticCount++] = ds;
        }
        // Handle international students
        else if (status == 'I' && requiresTOEFL) {
            if (fieldsRead != 6) {
                fprintf(output, "Error: Missing TOEFL score for international student\n");
                continue;
            }
            InternationalStudent is;
            strcpy(is.firstName, firstName);
            strcpy(is.lastName, lastName);
            strcpy(is.birthDigits, formattedBirthDigits);
            is.day = day;
            is.month = month;
            is.year = year;
            is.gpa = gpa;
            is.status = status;
            is.toefl = toefl;
            internationalList[internationalCount++] = is;
        }
    }

    // Sort and output based on the given option
    if (option == 1) {
        mergeSort(domesticList, 0, domesticCount - 1, sizeof(DomesticStudent), compareStudents);
        for (int i = 0; i < domesticCount; i++) {
            fprintf(output, "%s %s %.3f %s %c\n",
                    domesticList[i].firstName, domesticList[i].lastName,
                    domesticList[i].gpa, domesticList[i].birthDigits,
                    domesticList[i].status);
        }
    } else if (option == 2) {
        mergeSort(internationalList, 0, internationalCount - 1, sizeof(InternationalStudent), compareStudents);
        for (int i = 0; i < internationalCount; i++) {
            fprintf(output, "%s %s %.3f %s %c %d\n",
                    internationalList[i].firstName, internationalList[i].lastName,
                    internationalList[i].gpa, internationalList[i].birthDigits,
                    internationalList[i].status, internationalList[i].toefl);
        }
    } else if (option == 3) {
        mergeSort(domesticList, 0, domesticCount - 1, sizeof(DomesticStudent), compareStudents);
        mergeSort(internationalList, 0, internationalCount - 1, sizeof(InternationalStudent), compareStudents);

        for (int i = 0; i < domesticCount; i++) {
            fprintf(output, "%s %s %.3f %s %c\n",
                    domesticList[i].firstName, domesticList[i].lastName,
                    domesticList[i].gpa, domesticList[i].birthDigits,
                    domesticList[i].status);
        }
        for (int i = 0; i < internationalCount; i++) {
            fprintf(output, "%s %s %.3f %s %c %d\n",
                    internationalList[i].firstName, internationalList[i].lastName,
                    internationalList[i].gpa, internationalList[i].birthDigits,
                    internationalList[i].status, internationalList[i].toefl);
        }
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