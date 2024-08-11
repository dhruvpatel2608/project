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