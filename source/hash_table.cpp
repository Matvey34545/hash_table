#include <string.h>
#include <stdio.h>

#include "hash_table.h"

#define SIZE_CRC_TABLE 256

const int LOAD_FACTOR = 2;
const int CONST_REHASH = 2;
const int MIN_SIZE_TABLE = 100;

const char* END_LIST = "*";

static ListChain* find_elem(const HashTable* hash_table, const char* name, int number_cell);
static ErrorHash rehash_table(HashTable* hash_table);
static ErrorHash rehash_table_in(HashTable* hash_table, uint32_t resize);
static void insert_begin_list(ListChain** begin_list, ListChain* insert_elem);

static uint32_t crc32(const uint8_t *data, size_t length);
static void generate_crc32_table(uint32_t *table);

static ErrorHash rehash_table(HashTable* hash_table)
{
    if (hash_table->size >= hash_table->capacity * LOAD_FACTOR)
        return rehash_table_in(hash_table, hash_table->capacity * CONST_REHASH);

    if (hash_table->size <=  hash_table->capacity && hash_table->capacity > MIN_SIZE_TABLE)
        return rehash_table_in(hash_table, hash_table->capacity / CONST_REHASH);

    return HASH_ERROR_NO;
}

static ErrorHash rehash_table_in(HashTable* hash_table, uint32_t resize)
{
    HashTable new_table = {};
    new_table.ptr_table = (ListChain**)calloc(resize, sizeof(ListChain*));
    if (new_table.ptr_table == NULL)
        return HASH_ERROR_ALLOCATION;

    new_table.capacity = resize;
    new_table.size = hash_table->size;
    new_table.hash_func = hash_table->hash_func;

    for (int i = 0; i < hash_table->capacity; i++)
    {
        ListChain* ptr = hash_table->ptr_table[i];
        while (ptr != NULL)
        {
            ListChain* ptr_right = ptr->right;
            insert_begin_list(new_table.ptr_table + hash_crc_32(&new_table, ptr->name), ptr);
            ptr = ptr_right;
        } 
    }
    memcpy(hash_table, &new_table, sizeof(HashTable));

    return HASH_ERROR_NO;
}

ErrorHash insert_word(HashTable* hash_table, const char* name)
{
    ErrorHash error = rehash_table(hash_table);
    if (error != HASH_ERROR_NO)
        return error;

    int number_cell = hash_table->hash_func(hash_table, name);
    ListChain* elem_table = find_elem(hash_table, name, number_cell);
    if (elem_table != NULL)
    {
        elem_table->number_repeat += 1;
        return HASH_ERROR_NO;
    }

    ListChain* begin_create = (ListChain*)calloc(1, sizeof(ListChain));
    if (begin_create == NULL)
        return HASH_ERROR_ALLOCATION;

    insert_begin_list(hash_table->ptr_table + number_cell, begin_create);

    int size = strlen(name) + 1;
    begin_create->name = (char*)calloc(size, sizeof(char));
    if (begin_create->name == NULL)
        return HASH_ERROR_ALLOCATION;

    memcpy(begin_create->name, name, size * sizeof(char));
    
    hash_table->ptr_table[number_cell] = begin_create; 
    hash_table->size++;

    return HASH_ERROR_NO;
}

static void insert_begin_list(ListChain** begin_list, ListChain* insert_elem)
{
    insert_elem->left = NULL;
    if (*begin_list == NULL)
    {
        insert_elem->right = NULL;
        *begin_list = insert_elem;
        return;
    }
    (*begin_list)->left = insert_elem;
    insert_elem->right = (*begin_list);
    *begin_list = insert_elem;
}


ErrorHash extract_word(HashTable* hash_table, const char* name)
{
    ErrorHash error = rehash_table(hash_table);
    if (error != HASH_ERROR_NO)
        return error;


    int number_cell = hash_table->hash_func(hash_table, name);
    ListChain* elem_table = find_elem(hash_table, name, number_cell);

    if (elem_table == NULL)
        return HASH_NO_FIND;

    if (elem_table->number_repeat > 0)
    {
        elem_table->number_repeat--;
        return HASH_ERROR_NO;
    }

    ListChain* left = elem_table->left;
    ListChain* right = elem_table->right;

    if (left != NULL)
        left->right = right;
    else
        hash_table->ptr_table[number_cell] = right;

    if (right != NULL)
        right->left = left;

    hash_table->size--;
    free(elem_table);
    return HASH_ERROR_NO;
}

ListChain* find_word(const HashTable* hash_table, const char* name)
{
    int number_cell = hash_table->hash_func(hash_table, name);
    return find_elem(hash_table, name, number_cell);
}

ErrorHash ctor_hash_table(HashTable* hash_table, uint32_t begin_size)
{
    hash_table->ptr_table = (ListChain**)calloc(begin_size, sizeof(ListChain*));
    if (hash_table->ptr_table == NULL)
        return HASH_ERROR_ALLOCATION;

    hash_table->capacity = begin_size;
    hash_table->size = 0;
    hash_table->hash_func = hash_crc_32;

    return HASH_ERROR_NO;
}

void dtor_hash_table(HashTable* hash_table)
{
    for (int i = 0; i < hash_table->capacity; i++)
    {
        ListChain* ptr = hash_table->ptr_table[i];
        while (ptr != NULL)
        {
            ListChain* ptr_right = ptr->right;
            free(ptr->name);
            free(ptr);
            ptr = ptr_right;
        }
    }
}

ErrorHash write_hash_table(const HashTable* hash_table, const char* filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return HASH_ERROR_OPEN_FILE;

    fprintf(fp, "%d %d\n", hash_table->size, hash_table->capacity);
    for (int i = 0; i < hash_table->capacity; i++)
    {
        ListChain* ptr = hash_table->ptr_table[i];

        while (ptr != NULL)
        {
            fprintf(fp, "%s %d ", ptr->name, ptr->number_repeat);
            ptr = ptr->right;
        }
        fprintf(fp, "%s\n", END_LIST);
    }

    fclose(fp);
    return HASH_ERROR_NO;
}

ErrorHash read_hash_table(HashTable* hash_table, const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return HASH_ERROR_OPEN_FILE;

    fscanf(fp, "%d %d\n", &hash_table->size, &hash_table->capacity);
    hash_table->ptr_table = (ListChain**)calloc(hash_table->capacity, sizeof(ListChain*));
    if (hash_table->ptr_table == NULL)
        return HASH_ERROR_ALLOCATION;

    for (int i = 0; i < hash_table->capacity; i++)
    {
        ListChain* ptr = hash_table->ptr_table[i];
        ListChain* ptr_last = NULL;

        char name[40] = {};
        int number_repeat = 0;
        
        bool is_format = false;

        while (fscanf(fp, "%s", name) > 0)
        {
            if (strcmp(name, END_LIST) == 0)
            {
                is_format = true;
                break;
            }
            if (fscanf(fp, "%d", &number_repeat) == 0)
                return HASH_ERROR_READ_FORMAT;

            ptr = (ListChain*)calloc(1, sizeof(ListChain));
            if (ptr == NULL)
                return HASH_ERROR_ALLOCATION;

            if (ptr_last != NULL)
            {
                ptr_last->right = ptr;
                ptr->left = ptr_last;
            }
            else
                hash_table->ptr_table[i] = ptr; 

            int len_str = strlen(name) + 1;
            ptr->name = (char*)calloc(len_str, sizeof(char));
            if (ptr->name == NULL)
                return HASH_ERROR_ALLOCATION;

            ptr->number_repeat = number_repeat;
            memcpy(ptr->name, name, len_str * sizeof(char));

            ptr_last = ptr;
            ptr = ptr->right;
        }

        if (!is_format)
            return HASH_ERROR_READ_FORMAT;

        
    }

    fclose(fp);
    return HASH_ERROR_NO;
}

static ListChain* find_elem(const HashTable* hash_table, const char* name, int number_cell)
{
    ListChain* ptr = hash_table->ptr_table[number_cell];

    while (ptr != NULL)
    {
        if (strcmp(name, ptr->name) == 0)
            return ptr;

        ptr = ptr->right;
    }
    return NULL;
}

uint32_t hash_crc_32(const HashTable* hash_table, const char* name)
{
    return crc32((const uint8_t*)name, strlen(name)) % hash_table->capacity;
}

static void generate_crc32_table(uint32_t *table) 
{
    for (uint32_t i = 0; i < 256; i++) 
    {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) 
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
        table[i] = crc;
    }
}

static uint32_t crc32(const uint8_t *data, size_t length) 
{
    static uint32_t table[SIZE_CRC_TABLE];
    static bool table_generated = false;
    
    if (!table_generated) 
    {
        generate_crc32_table(table);
        table_generated = true;
    }

    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < length; i++) 
    {
        uint8_t byte = data[i];
        crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
    }
    
    return crc ^ 0xFFFFFFFF;
}