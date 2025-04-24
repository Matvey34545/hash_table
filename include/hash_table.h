#ifndef HASH_TABLE
#define HASH_TABLE

#include <stdlib.h>
#include <stdint.h>

struct ListChain
{
    ListChain* left;
    ListChain* right;
    char* name;
    int number_repeat;
};

struct HashTable
{
    ListChain** ptr_table;
    uint32_t size;
    uint32_t capacity;
    uint32_t (*hash_func)(const HashTable*, const char*);
};

enum ErrorHash
{
    HASH_ERROR_NO          = 0,
    HASH_ERROR_ALLOCATION  = 1,
    HASH_NO_FIND           = 2,
    HASH_ERROR_OPEN_FILE   = 3,
    HASH_ERROR_READ_FORMAT = 4
};

uint32_t hash_crc_32(const HashTable* hash_table, const char* name);
ErrorHash insert_word(HashTable* hash_table, const char* name);
ErrorHash extract_word(HashTable* hash_table, const char* name);
ErrorHash ctor_hash_table(HashTable* hash_table, uint32_t begin_size);
ListChain* find_word(const HashTable* hash_table, const char* name);
void dtor_hash_table(HashTable* hash_table);
ErrorHash write_hash_table(const HashTable* hash_table, const char* filename);
ErrorHash read_hash_table(HashTable* hash_table, const char* filename);

#endif