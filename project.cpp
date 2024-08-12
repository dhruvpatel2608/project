#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


#pragma warning(disable:4996)

#define HASH_TABLE_SIZE 127

typedef struct Parcel {
    char* destination;
    int weight;
    float valuation;
    struct Parcel* left;
    struct Parcel* right;
} Parcel;

typedef struct HashTable {
    Parcel* table[HASH_TABLE_SIZE];
} HashTable;

void toLowerCase(char* str) {
    for (; *str; ++str) {
        *str = tolower((unsigned char)*str);
    }
}


unsigned long djb2_hash(const char* str);
Parcel* createParcel(const char* destination, int weight, float valuation);
void insertParcel(Parcel** root, const char* destination, int weight, float valuation);
Parcel* searchParcel(Parcel* root, int weight);
Parcel* searchParcelByDestination(Parcel* root, const char* destination);
void printParcel(Parcel* parcel);
void printAllParcels(Parcel* root);
void printParcelsWithCondition(Parcel* root, int weight, int condition);
HashTable* createHashTable();
void freeParcel(Parcel* parcel);
void totalLoadAndValuation(Parcel* root, int* totalLoad, float* totalValuation);
Parcel* findMin(Parcel* root);
Parcel* findMax(Parcel* root);
int handleCountryName(char* country, unsigned long* hashIndex, HashTable* hashTable);
int isCountryInHashTable(Parcel* root, const char* country);
void handleWeightInput(int* weight);
void handleConditionInput(int* condition);
void handleUserMenu(HashTable* hashTable);

int main() {
    HashTable* hashTable = createHashTable();
    if (hashTable == NULL) {
        return 1;
    }

    FILE* file;
    errno_t err = fopen_s(&file, "couries.txt", "r");
    if (err != 0 || file == NULL) {
        printf("Error opening file\n");
        free(hashTable);
        return 1;
    }

    char line[128]; // Adjust the size based on the maximum expected line length
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Remove newline character

        char* destination = line;
        char* weightStr = strchr(line, ',');
        if (weightStr) {
            *weightStr = '\0';
            weightStr++;
        }
        char* valuationStr = weightStr ? strchr(weightStr, ',') : NULL;
        if (valuationStr) {
            *valuationStr = '\0';
            valuationStr++;
        }

        if (destination && weightStr && valuationStr) {
            int weight = atoi(weightStr);
            float valuation = (float)atof(valuationStr);

            unsigned long hashIndex = djb2_hash(destination);
            insertParcel(&hashTable->table[hashIndex], destination, weight, valuation);
        }
        else {
            printf("Malformed line in file: %s\n", line);
        }
    }

    if (fclose(file) != 0) {
        printf("Error closing file\n");
    }

    handleUserMenu(hashTable);

    return 0;
}

unsigned long djb2_hash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++) != '\0') {
        c = tolower(c);  // Convert to lowercase for consistent hashing
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}


Parcel* createParcel(const char* destination, int weight, float valuation) {
    Parcel* newParcel = (Parcel*)malloc(sizeof(Parcel));
    if (newParcel == NULL) {
        printf("Failed to allocate memory for new parcel\n");
        return NULL;
    }
    newParcel->destination = (char*)malloc(strlen(destination) + 1);
    if (newParcel->destination == NULL) {
        printf("Failed to allocate memory for destination\n");
        free(newParcel);
        return NULL;
    }
    strcpy_s(newParcel->destination, strlen(destination) + 1, destination);
    toLowerCase(newParcel->destination);  // Convert to lowercase
    newParcel->weight = weight;
    newParcel->valuation = valuation;
    newParcel->left = newParcel->right = NULL;
    return newParcel;
}

void insertParcel(Parcel** root, const char* destination, int weight, float valuation) {
    if (*root == NULL) {
        *root = createParcel(destination, weight, valuation);
        return;
    }
    if (weight < (*root)->weight) {
        insertParcel(&(*root)->left, destination, weight, valuation);
    }
    else {
        insertParcel(&(*root)->right, destination, weight, valuation);
    }
}

Parcel* searchParcel(Parcel* root, int weight) {
    if (root == NULL || root->weight == weight) {
        return root;
    }
    if (weight < root->weight) {
        return searchParcel(root->left, weight);
    }
    else {
        return searchParcel(root->right, weight);
    }
}

Parcel* searchParcelByDestination(Parcel* root, const char* destination) {
    if (root == NULL) {
        return NULL;
    }

    // Allocate memory for lowerDestination
    char* lowerDestination = (char*)malloc(strlen(destination) + 1);
    if (lowerDestination == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    strcpy_s(lowerDestination, strlen(destination) + 1, destination);
    toLowerCase(lowerDestination);  // Convert to lowercase

    Parcel* foundParcel = NULL;
    if (strcmp(root->destination, lowerDestination) == 0) {
        foundParcel = root;
    }
    else {
        foundParcel = searchParcelByDestination(root->left, destination);
        if (foundParcel == NULL) {
            foundParcel = searchParcelByDestination(root->right, destination);
        }
    }

    // Free the allocated memory
    free(lowerDestination);

    return foundParcel;
}

void printParcel(Parcel* parcel) {
    if (parcel) {
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n", parcel->destination, parcel->weight, parcel->valuation);
    }
    else {
        printf("Parcel not found.\n");
    }
}

void printAllParcels(Parcel* root, const char* country) {
    if (root == NULL) {
        return;
    }
    printAllParcels(root->left, country);

    // Print only if the parcel's destination matches the country
    if (strcmp(root->destination, country) == 0) {
        printParcel(root);
    }

    printAllParcels(root->right, country);
}


void printParcelsWithCondition(Parcel* root, int weight, int condition, const char* country) {
    if (root == NULL) {
        return;
    }

    // Print parcels based on the weight condition and matching the country
    if (strcmp(root->destination, country) == 0 &&
        ((condition == 1 && root->weight > weight) ||
            (condition == 0 && root->weight < weight))) {
        printParcel(root);
    }

    // Recursively check left and right subtrees
    printParcelsWithCondition(root->left, weight, condition, country);
    printParcelsWithCondition(root->right, weight, condition, country);
}


HashTable* createHashTable() {
    HashTable* hashTable = (HashTable*)malloc(sizeof(HashTable));
    if (hashTable == NULL) {
        printf("Failed to allocate memory for hash table\n");
        return NULL;
    }
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        hashTable->table[i] = NULL;
    }
    return hashTable;
}

void freeParcel(Parcel* parcel) {
    if (parcel) {
        free(parcel->destination);
        freeParcel(parcel->left);
        freeParcel(parcel->right);
        free(parcel);
    }
}

void totalLoadAndValuation(Parcel* root, int* totalLoad, float* totalValuation) {
    if (root == NULL) {
        return;
    }
    *totalLoad += root->weight;
    *totalValuation += root->valuation;
    totalLoadAndValuation(root->left, totalLoad, totalValuation);
    totalLoadAndValuation(root->right, totalLoad, totalValuation);
}

Parcel* findMin(Parcel* root) {
    while (root && root->left != NULL) {
        root = root->left;
    }
    return root;
}

Parcel* findMax(Parcel* root) {
    while (root && root->right != NULL) {
        root = root->right;
    }
    return root;
}

int handleCountryName(char* country, unsigned long* hashIndex, HashTable* hashTable) {
    printf("Enter country name: ");
    if (fgets(country, 21, stdin) == NULL) {  // Use the correct buffer size
        printf("Error reading country name.\n");
        return 0;
    }
    country[strcspn(country, "\n")] = '\0';  // Remove newline character
    toLowerCase(country);  // Convert the input to lowercase
    *hashIndex = djb2_hash(country);

    // Check if the country exists in the hash table at the given index
    return isCountryInHashTable(hashTable->table[*hashIndex], country);
}


int isCountryInHashTable(Parcel* root, const char* country) {
    return searchParcelByDestination(root, country) != NULL;
}

void handleWeightInput(int* weight, int* success) {
    printf("Enter weight: ");
    if (scanf_s("%d", weight) == 1) {
        *success = 1;  // Valid input received
    }
    else {
        printf("Invalid input for weight. Please enter a number.\n");
        *success = 0;  // Invalid input
    }
    while (getchar() != '\n');  // Clear any leftover characters in the input buffer
}



void handleConditionInput(int* condition) {
    printf("Enter 1 for higher or 0 for lower: ");
    if (scanf_s("%d", condition) != 1 || (*condition != 0 && *condition != 1)) {
        printf("Invalid input. Please enter 0 or 1.\n");
        while (getchar() != '\n'); // Clear input buffer
    }
    while (getchar() != '\n'); // Clear input buffer
}

void handleUserMenu(HashTable* hashTable) {
    int choice;
    char inputBuffer[10];
    char country[21];
    int weight;
    int condition;
    unsigned long hashIndex;
    Parcel* foundParcel;
    int totalLoad;
    float totalValuation;
    Parcel* minParcel;
    Parcel* maxParcel;
    Parcel* lightestParcel;
    Parcel* heaviestParcel;
    int weightInputSuccess;

    while (1) {
        printf("User Menu:\n");
        printf("1. Enter country name and display all the parcels details\n");
        printf("2. Enter country and weight pair\n");
        printf("3. Display the total parcel load and valuation for the country\n");
        printf("4. Enter the country name and display cheapest and most expensive parcel's details\n");
        printf("5. Enter the country name and display lightest and heaviest parcel for the country\n");
        printf("6. Exit the application\n");
        printf("Enter your choice: ");

        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || inputBuffer[0] == '\n') {
            printf("Invalid input. Please select a valid menu option.\n");
            continue;
        }

        choice = atoi(inputBuffer);

        switch (choice) {
        case 1:
            if (handleCountryName(country, &hashIndex, hashTable)) {
                if (hashTable->table[hashIndex] != NULL) {
                    printAllParcels(hashTable->table[hashIndex], country);
                }
                else {
                    printf("Country '%s' not found in the list.\n", country);
                }
            }
            else {
                printf("Country '%s' not found in the list.\n", country);
            }
            break;

        case 2:
            if (handleCountryName(country, &hashIndex, hashTable)) {
                if (hashTable->table[hashIndex] != NULL) {
                    handleWeightInput(&weight, &weightInputSuccess);
                    if (weightInputSuccess) {
                        handleConditionInput(&condition);
                        printParcelsWithCondition(hashTable->table[hashIndex], weight, condition, country);
                    }
                }
                else {
                    printf("Country '%s' not found in the list.\n", country);
                }
            }
            else {
                printf("Country '%s' not found in the list.\n", country);
            }
            break;

        case 3:
            if (handleCountryName(country, &hashIndex, hashTable)) {
                if (hashTable->table[hashIndex] != NULL && strcmp(hashTable->table[hashIndex]->destination, country) == 0) {
                    totalLoad = 0;
                    totalValuation = 0.0f;
                    totalLoadAndValuation(hashTable->table[hashIndex], &totalLoad, &totalValuation);
                    printf("Total Load: %d, Total Valuation: %.2f\n", totalLoad, totalValuation);
                }
                else {
                    printf("Country '%s' not found in the list.\n", country);
                }
            }
            else {
                printf("Country '%s' not found in the list.\n", country);
            }
            break;

        case 4:
            if (handleCountryName(country, &hashIndex, hashTable)) {
                if (hashTable->table[hashIndex] != NULL && strcmp(hashTable->table[hashIndex]->destination, country) == 0) {
                    minParcel = findMin(hashTable->table[hashIndex]);
                    maxParcel = findMax(hashTable->table[hashIndex]);
                    printf("Cheapest Parcel:\n");
                    printParcel(minParcel);
                    printf("Most Expensive Parcel:\n");
                    printParcel(maxParcel);
                }
                else {
                    printf("Country '%s' not found in the list.\n", country);
                }
            }
            else {
                printf("Country '%s' not found in the list.\n", country);
            }
            break;

        case 5:
            if (handleCountryName(country, &hashIndex, hashTable)) {
                if (hashTable->table[hashIndex] != NULL && strcmp(hashTable->table[hashIndex]->destination, country) == 0) {
                    lightestParcel = findMin(hashTable->table[hashIndex]);
                    heaviestParcel = findMax(hashTable->table[hashIndex]);
                    printf("Lightest Parcel:\n");
                    printParcel(lightestParcel);
                    printf("Heaviest Parcel:\n");
                    printParcel(heaviestParcel);
                }
                else {
                    printf("Country '%s' not found in the list.\n", country);
                }
            }
            else {
                printf("Country '%s' not found in the list.\n", country);
            }
            break;

        case 6:
            for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
                freeParcel(hashTable->table[i]);
            }
            free(hashTable);
            return;
        default:
            printf("Invalid choice. Please select a valid menu option.\n");
        }
    }
}
