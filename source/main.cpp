#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "hash_table.h"

struct File
{
    char* ptr;
    int size;
};

const char* NAME_FILE_FOR_TEST_READ_WRITE = "dump.txt";
const int NUMBER_FIND = (int)1e4;
const int BEGIN_CAPACITY = 100;

ErrorHash testing_hash(const File* file);
ErrorHash read_file(const char* filename, File* file);
bool probability_choice(int prob_true, int prob_false);

int main(int argc, char* argv[])
{
    char* filename = argv[1];
    char* filename_test = argv[2];

    File file = {};
    ErrorHash error = read_file(filename_test, &file);
    if (error != HASH_ERROR_NO)
        return error;

    HashTable hash_table = {};
    ctor_hash_table(&hash_table, BEGIN_CAPACITY);

    clock_t start = clock();
    testing_hash(&file);
    clock_t end = clock();

    FILE* fp = fopen(filename, "a");
    if (fp == NULL)
        return HASH_ERROR_OPEN_FILE;

    fprintf(fp, "%lf\n", (double)(end - start) / CLOCKS_PER_SEC * 1e3);
    fclose(fp);
}

ErrorHash read_file(const char* filename, File* file)
{
    struct stat st = {};
    stat(filename, &st);
    file->size = st.st_size + 1;

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        return HASH_ERROR_OPEN_FILE;

    file->ptr = (char*)calloc(file->size, sizeof(char));

    if (file->ptr == NULL)
        return HASH_ERROR_ALLOCATION;

    fread(file->ptr, sizeof(char), file->size - 1, fp);
    return HASH_ERROR_NO;
}

ErrorHash testing_hash(const File* file)
{
    File new_file = {file->ptr, 0};

    HashTable hash_table = {};
    ctor_hash_table(&hash_table, BEGIN_CAPACITY);

    while (new_file.size < file->size)
    {
        while (isspace(*(new_file.ptr)) && *(new_file.ptr) != '\0')
            new_file.ptr++;

        if (*(new_file.ptr) == '\0')
            break;

        char* name = new_file.ptr;

        while(!isspace(*(new_file.ptr)) && *(new_file.ptr) != '\0')
            new_file.ptr++;

        *new_file.ptr = '\0';
        new_file.ptr++;

        insert_word(&hash_table, name);
    }

    for (int i = 0; i < NUMBER_FIND; i++)
    {
        char* name = file->ptr + rand() % file->size - 1;
        while (isspace(*name) && *name != '\0')
            name++;
        

        bool operation = probability_choice(95, 5);

        if (operation)
            find_word(&hash_table, name);

        else
            extract_word(&hash_table, name);
    }

    write_hash_table(&hash_table, NAME_FILE_FOR_TEST_READ_WRITE);
    return HASH_ERROR_NO;
}

bool probability_choice(int prob_true, int prob_false)
{
    int random_value = rand() % 100;
    return random_value < prob_true;
}

