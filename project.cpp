#include <stdio.h>


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


unsigned long djb2_hash(char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++) != NULL) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

Parcel* insertParcel(Parcel** root, const char* destination, int weight, float valuation) {
    Parcel* newParcel = (Parcel*)malloc(sizeof(Parcel));
    if (!newParcel) {
        fprintf(stderr, "Error allocating memory for newParcel.\n");
        return NULL;
    }

    newParcel->destination = (char*)malloc((strlen(destination) + 1) * sizeof(char));
    if (!newParcel->destination) {
        fprintf(stderr, "Error allocating memory for destination.\n");
        free(newParcel);
        return NULL;
    }

    strcpy_s(newParcel->destination, strlen(destination) + 1, destination);
    newParcel->weight = weight;
    newParcel->valuation = valuation;
    newParcel->left = newParcel->right = NULL;

    if (*root == NULL) {
        *root = newParcel;
    } else {
        Parcel* current = *root;
        Parcel* parent = NULL;
        while (current != NULL) {
            parent = current;
            if (weight < current->weight) {
                current = current->left;
            } else {
                current = current->right;
            }
        }

        if (weight < parent->weight) {
            parent->left = newParcel;
        } else {
            parent->right = newParcel;
        }
    }

    return newParcel;
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

void printParcel(Parcel* parcel) {
    if (parcel) {
        printf("Destination: %s, Weight: %d, Valuation: %.2f\n", parcel->destination, parcel->weight, parcel->valuation);
    }
    else {
        printf("Parcel not found.\n");
    }
}

Parcel* searchParcelByDestination(Parcel* root, const char* destination) {
    if (root == NULL) {
        return NULL;
    }
    if (strcmp(root->destination, destination) == 0) {
        return root;
    }
    Parcel* found = searchParcelByDestination(root->left, destination);
    if (found != NULL) {
        return found;
    }
    return searchParcelByDestination(root->right, destination);
}

void printAllParcels(Parcel* root) {
    if (root == NULL) {
        return;
    }
    printAllParcels(root->left);
    printParcel(root);
    printAllParcels(root->right);
}

Void printParcelsWithCondition(Parcel* root, int weight, int condition) {
    if (root == NULL) {
        return;
    }

    // Print parcels based on the weight condition
    if ((condition == 1 && root->weight > weight) ||
        (condition == 0 && root->weight < weight)) {
        printParcel(root);
    }

    // Recursively check left and right subtrees
    printParcelsWithCondition(root->left, weight, condition);
    printParcelsWithCondition(root->right, weight, condition);
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
    if (fgets(country, sizeof(country), stdin) == NULL) {
        printf("Error reading country name.\n");
        return 0;
    }
    country[strcspn(country, "\n")] = '\0'; // Remove newline character
    *hashIndex = djb2_hash(country);

    // Check if the country exists in the hash table at the given index
    return isCountryInHashTable(hashTable->table[*hashIndex], country);
}

int isCountryInHashTable(Parcel* root, const char* country) {
    return searchParcelByDestination(root, country) != NULL;
}

void handleWeightInput(int* weight) {
    printf("Enter weight: ");
    if (scanf_s("%d", weight) != 1) {
        printf("Invalid input for weight. Please enter a number.\n");
        while (getchar() != '\n'); // Clear input buffer
    }
    while (getchar() != '\n'); // Clear input buffer
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

    while (1) {
        printf("User Menu:\n");
        printf("1. Enter country name and display all the parcels details\n");
        printf("2. Enter country and weight pair\n");
        printf("3. Display the total parcel load and valuation for the country\n");
        printf("4. Enter the country name and display cheapest and most expensive parcel's details\n");
        printf("5. Enter the country name and display lightest and heaviest parcel for the country\n");
        printf("6. Exit the application\n");
        printf("Enter your choice: ");

        // Get the entire input line as a string
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL || inputBuffer[0] == '\n') {
            printf("Invalid input. Please select a valid menu option.\n");
            continue;
        }

        // Convert input string to an integer
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
                    handleWeightInput(&weight);
                    handleConditionInput(&condition);
                    printParcelsWithCondition(hashTable->table[hashIndex], weight, condition, country);
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
                hashTable->table[hashIndex] != NULL && strcmp(hashTable->table[hashIndex]->destination, country) == 0;
                totalLoad = 0;
                totalValuation = 0.0f;
                totalLoadAndValuation(hashTable->table[hashIndex], &totalLoad, &totalValuation);
                printf("Total Load: %d, Total Valuation: %.2f\n", totalLoad, totalValuation);

            }
            else {
                printf("Country '%s' not found in the list.\n", country);
            }
            break;
        case 4:
            if (handleCountryName(country, &hashIndex, hashTable)) {
                hashTable->table[hashIndex] != NULL && strcmp(hashTable->table[hashIndex]->destination, country) == 0;
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
            break;
        case 5:
            if (handleCountryName(country, &hashIndex, hashTable)) {
                hashTable->table[hashIndex] != NULL && strcmp(hashTable->table[hashIndex]->destination, country) == 0;
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