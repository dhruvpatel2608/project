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

unsigned long djb2_hash(const char* str);
Parcel* createParcel(const char* destination, int weight, float valuation);
void insertParcel(Parcel** root, const char* destination, int weight, float valuation);
Parcel* searchParcel(Parcel* root, int weight);
Parcel* searchParcelByDestination(Parcel* root, const char* destination);
void printParcel(Parcel* parcel);
void printAllParcels(Parcel* root);
void printParcelsWithCondition(Parcel* root, int weight, int condition);
HashTable* createHashTable();
void clean(HashTable* hashTable);
void totalLoadAndValuation(Parcel* root, int* totalLoad, float* totalValuation);
Parcel* findMin(Parcel* root);
Parcel* findMax(Parcel* root);
int handleCountryName(char* country, unsigned long* hashIndex, HashTable* hashTable);
int isCountryInHashTable(Parcel* root, const char* country);
void handleWeightInput(int* weight);
void handleConditionInput(int* condition);
void handleUserMenu(HashTable* hashTable);

/*
 * FUNCTION: main
 * DESCRIPTION: Main entry point of the program. Initializes the hash table, reads parcel data from a file,
 *              inserts parcels into the hash table, and then presents a user menu for interaction with the data.
 * PARAMETERS: None.
 * RETURNS: int - Exit status code:
 *         - 0 if the program completes successfully.
 *         - 1 if there is an error in creating the hash table or opening/closing the file.
 */
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

/*
 * FUNCTION: djb2_hash
 * DESCRIPTION: Computes a hash value for a given string using the djb2 hash function.
 * PARAMETERS: const char* str - The string to hash.
 * RETURNS: The computed hash value as an unsigned long integer.
 */
unsigned long djb2_hash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++) != '\0') {
        c = tolower(c);  // Convert to lowercase for consistent hashing
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

/*
 * FUNCTION: createParcel
 * DESCRIPTION: Creates a new parcel with the specified destination, weight, and valuation.
 * PARAMETERS: const char* destination - The destination of the parcel.
 *             int weight - The weight of the parcel.
 *             float valuation - The valuation of the parcel.
 * RETURNS: A pointer to the newly created Parcel.
 */
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

    // Convert destination to lowercase here
    for (char* p = newParcel->destination; *p; ++p) {
        *p = tolower((unsigned char)*p);
    }

    newParcel->weight = weight;
    newParcel->valuation = valuation;
    newParcel->left = newParcel->right = NULL;
    return newParcel;
}

/*
*FUNCTION: insertParcel
* DESCRIPTION : Inserts a new parcel into a binary search tree(BST) based on weight.
* PARAMETERS : Parcel * *root - Pointer to the root of the BST.
* const char* destination - The destination of the parcel.
* int weight - The weight of the parcel.
* float valuation - The valuation of the parcel.
* RETURNS : None.
*/
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

/*
 * FUNCTION: searchParcel
 * DESCRIPTION: Searches for a parcel in the BST by weight.
 * PARAMETERS: Parcel* root - The root of the BST.
 *             int weight - The weight to search for.
 * RETURNS: A pointer to the Parcel if found, otherwise NULL.
 */

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

/*
 * FUNCTION: searchParcelByDestination
 * DESCRIPTION: Searches for a parcel in the BST by destination.
 * PARAMETERS: Parcel* root - The root of the BST.
 *             const char* destination - The destination to search for.
 * RETURNS: A pointer to the Parcel if found, otherwise NULL.
 */

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

    // Convert lowerDestination to lowercase here
    for (char* p = lowerDestination; *p; ++p) {
        *p = tolower((unsigned char)*p);
    }

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

/*
 * FUNCTION: printParcel
 * DESCRIPTION: Prints the details of a single parcel.
 * PARAMETERS: Parcel* parcel - The parcel to print.
 * RETURNS: None.
 */

void printParcel(Parcel* parcel) {
    if (parcel) {
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n\n", parcel->destination, parcel->weight, parcel->valuation);
    }
    else {
        printf("Parcel not found.\n\n");
    }
}

/*
 * FUNCTION: printAllParcels
 * DESCRIPTION: Prints details of all parcels in the BST that match the given country.
 * PARAMETERS: Parcel* root - The root of the BST.
 *             const char* country - The country to match.
 * RETURNS: None.
 */

void printAllParcels(Parcel* root, const char* country) {
    if (root == NULL) {
        return;
    }
    printAllParcels(root->left, country);

    // Convert country to lowercase and compare
    char lowerCountry[21];
    strcpy_s(lowerCountry, sizeof(lowerCountry), country);
    for (char* p = lowerCountry; *p; ++p) {
        *p = tolower((unsigned char)*p);
    }

    if (strcmp(root->destination, lowerCountry) == 0) {
        printParcel(root);
    }

    printAllParcels(root->right, country);
}

/*
 * FUNCTION: printParcelsWithCondition
 * DESCRIPTION: Prints parcels that meet the specified weight condition and match the given country.
 * PARAMETERS: Parcel* root - The root of the BST.
 *             int weight - The weight to compare against.
 *             int condition - The condition (1 for higher, 0 for lower).
 *             const char* country - The country to match.
 * RETURNS: None.
 */

void printParcelsWithCondition(Parcel* root, int weight, int condition, const char* country) {
    if (root == NULL) {
        return;
    }

    // Convert country to lowercase and compare
    char lowerCountry[21];
    strcpy_s(lowerCountry, sizeof(lowerCountry), country);
    for (char* p = lowerCountry; *p; ++p) {
        *p = tolower((unsigned char)*p);
    }

    // Print parcels based on the weight condition and matching the country
    if (strcmp(root->destination, lowerCountry) == 0 &&
        ((condition == 1 && root->weight > weight) ||
            (condition == 0 && root->weight < weight))) {
        printParcel(root);
    }

    // Recursively check left and right subtrees
    printParcelsWithCondition(root->left, weight, condition, country);
    printParcelsWithCondition(root->right, weight, condition, country);
}

/*
 * FUNCTION: createHashTable
 * DESCRIPTION: Creates a new hash table with initialized entries.
 * PARAMETERS: None.
 * RETURNS: A pointer to the newly created HashTable.
 */
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

/*
 * FUNCTION: clean
 * DESCRIPTION: Frees all memory associated with the hash table and its parcels.
 * PARAMETERS: HashTable* hashTable - The hash table to clean.
 * RETURNS: None.
 */
void clean(HashTable* hashTable) {
    // Free the parcels using a helper function
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        Parcel* parcel = hashTable->table[i];
        // Traverse the BST to free all parcels
        while (parcel) {
            Parcel* next = parcel->right;
            free(parcel->destination);
            free(parcel);
            parcel = next;
        }
    }
    free(hashTable);
}

/*
 * FUNCTION: totalLoadAndValuation
 * DESCRIPTION: Calculates the total load and valuation of all parcels in the BST.
 * PARAMETERS: Parcel* root - The root of the BST.
 *             int* totalLoad - Pointer to store the total load.
 *             float* totalValuation - Pointer to store the total valuation.
 * RETURNS: None.
 */
void totalLoadAndValuation(Parcel* root, int* totalLoad, float* totalValuation) {
    if (root == NULL) {
        return;
    }
    *totalLoad += root->weight;
    *totalValuation += root->valuation;
    totalLoadAndValuation(root->left, totalLoad, totalValuation);
    totalLoadAndValuation(root->right, totalLoad, totalValuation);
}

/*
 * FUNCTION: findMin
 * DESCRIPTION: Finds the parcel with the minimum weight in the BST.
 * PARAMETERS: Parcel* root - The root of the BST.
 * RETURNS: A pointer to the Parcel with the minimum weight.
 */
Parcel* findMin(Parcel* root) {
    while (root && root->left != NULL) {
        root = root->left;
    }
    return root;
}

/*
 * FUNCTION: findMax
 * DESCRIPTION: Finds the parcel with the maximum weight in the BST.
 * PARAMETERS: Parcel* root - The root of the BST.
 * RETURNS: A pointer to the Parcel with the maximum weight.
 */
Parcel* findMax(Parcel* root) {
    while (root && root->right != NULL) {
        root = root->right;
    }
    return root;
}

/*
 * FUNCTION: handleCountryName
 * DESCRIPTION: Prompts the user to enter a country name and computes its hash index.
 * PARAMETERS: char* country - Buffer to store the country name.
 *             unsigned long* hashIndex - Pointer to store the computed hash index.
 *             HashTable* hashTable - The hash table to check for the country.
 * RETURNS: 1 if the country exists in the hash table, 0 otherwise.
 */
int handleCountryName(char* country, unsigned long* hashIndex, HashTable* hashTable) {
    printf("Enter country name: ");
    if (fgets(country, 21, stdin) == NULL) {  // Use the correct buffer size
        printf("Error reading country name.\n\n");
        return 0;
    }
    country[strcspn(country, "\n")] = '\0';  // Remove newline character

    // Convert country to lowercase here
    for (char* p = country; *p; ++p) {
        *p = tolower((unsigned char)*p);
    }

    *hashIndex = djb2_hash(country);

    // Check if the country exists in the hash table at the given index
    return isCountryInHashTable(hashTable->table[*hashIndex], country);
}

/*
 * FUNCTION: isCountryInHashTable
 * DESCRIPTION: Checks if a country is present in the hash table.
 * PARAMETERS: Parcel* root - The root of the BST in the hash table bucket.
 *             const char* country - The country to check.
 * RETURNS: 1 if the country is found, 0 otherwise.
 */
int isCountryInHashTable(Parcel* root, const char* country) {
    return searchParcelByDestination(root, country) != NULL;
}

/*
 * FUNCTION: handleWeightInput
 * DESCRIPTION: Prompts the user to enter a weight value and validates the input.
 * PARAMETERS: int* weight - Pointer to store the entered weight.
 *             int* success - Pointer to indicate whether the input was successful.
 * RETURNS: None.
 */
void handleWeightInput(int* weight, int* success) {
    printf("Enter weight: ");
    if (scanf_s("%d", weight) == 1) {
        *success = 1;  // Valid input received
    }
    else {
        printf("Invalid input for weight. Please enter a number.\n\n");
        *success = 0;  // Invalid input
    }
    while (getchar() != '\n');  // Clear any leftover characters in the input buffer
}

/*
 * FUNCTION: handleConditionInput
 * DESCRIPTION: Prompts the user to enter a condition (1 for higher, 0 for lower) and validates the input.
 * PARAMETERS: int* condition - Pointer to store the entered condition.
 * RETURNS: None.
 */
void handleConditionInput(int* condition) {
    printf("Enter 1 for higher or 0 for lower: ");
    if (scanf_s("%d", condition) != 1 || (*condition != 0 && *condition != 1)) {
        printf("Invalid input. Please enter 0 or 1.\n");
        while (getchar() != '\n'); // Clear input buffer
    }
    while (getchar() != '\n'); // Clear input buffer
}

/*
 * FUNCTION: handleUserMenu
 * DESCRIPTION: Displays the user menu and handles user input for various operations on the hash table.
 * PARAMETERS: HashTable* hashTable - The hash table to operate on.
 * RETURNS: None.
 */
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
        printf("\nUser Menu:\n");
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
            clean(hashTable);
            return;
        default:
            printf("Invalid choice. Please select a valid menu option.\n");
        }
    }
}