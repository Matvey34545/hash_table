#include <stdio.h>
#include <time.h>

#include "hash_table.h"

enum NumberCommand
{
    INSERT  = 0,
    FIND    = 1,
    EXTRACT = 2,
    WRITE   = 3,
    READ    = 4
};

const char* NAME_FILE_FOR_TEST_READ_WRITE = "dump.txt";
const int NUMBER_COMMAND = (int)1e5;
const int BEGIN_CAPACITY = 100;

int main(int argc, char* argv[])
{
    char* filename = argv[1];
    FILE* fp = fopen(filename, "a");
    if (fp == NULL)
        return 1;

    int command = 0;
    char name[40] = {};

    HashTable hash_table = {};
    ctor_hash_table(&hash_table, BEGIN_CAPACITY);

    clock_t start = clock();
    for (int i = 0; i < NUMBER_COMMAND; i++)
    {
        scanf("%d", &command);
        switch (command)
        {
            case INSERT:
                scanf("%s", name);
                insert_word(&hash_table, name);
                break;

            case FIND:
                scanf("%s", name);
                find_word(&hash_table, name);
                break;

            case EXTRACT:
                scanf("%s", name);
                extract_word(&hash_table, name);
                break;

            case WRITE:
                write_hash_table(&hash_table, NAME_FILE_FOR_TEST_READ_WRITE);
                break;

                
            case READ:
                dtor_hash_table(&hash_table);
                read_hash_table(&hash_table, NAME_FILE_FOR_TEST_READ_WRITE);
                break;
            
        }
    }
    clock_t end = clock();

    fprintf(fp, "%lf\n", (double)(end - start) / CLOCKS_PER_SEC * 1e3);
    fclose(fp);
}

