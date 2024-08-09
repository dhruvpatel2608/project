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
