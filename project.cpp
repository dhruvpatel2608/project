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