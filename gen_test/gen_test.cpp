#include <stdio.h>
#include <stdlib.h>

const int NUMBER_COMMAND = (int)1e6;

const int PRINTABLE_START = 65;   
const int PRINTABLE_END = 122; 

#define MAX_LENGHT 20

int random_number_with_probabilities(const int numbers[], const double probabilities[], int size);
void gen_random_str(char* name);

int main()
{
    const int numbers[] = {0, 1, 2, 3, 4, 5};
    const double probabilities[] = {0.32, 0.32, 0.32,  0,02, 0,02};

    char name[MAX_LENGHT + 1] = {};
    for (int i = 0; i < NUMBER_COMMAND; i++)
    {
        int random_number = random_number_with_probabilities(numbers, probabilities, sizeof(numbers) / sizeof(int));
        printf("%d ", random_number);

        if (random_number <= 3)
        {
            gen_random_str(name);
            printf("%s", name);
        }
        printf("\n");
    }
}

int random_number_with_probabilities(const int numbers[], const double probabilities[], int size) 
{
    double r = (double)rand() / RAND_MAX; 
    double cumulative_prob = 0.0;

    for (int i = 0; i < size; i++) 
    {
        cumulative_prob += probabilities[i];
        if (r <= cumulative_prob)
            return numbers[i];
    }
    return numbers[size - 1]; 
}

void gen_random_str(char* name)
{
    int length = 1 + rand() % MAX_LENGHT;

    for (int j = 0; j < length; j++)
        name[j] = PRINTABLE_START + rand() % (PRINTABLE_END - PRINTABLE_START + 1);

    name[length] = '\0';
}