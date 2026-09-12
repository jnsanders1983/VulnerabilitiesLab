#include <stdio.h>

void normal(void)
{
    printf("Normal function executed.\n");
}

void win(void)
{
    printf("SUCCESS: control flow reached win().\n");
}

struct Challenge
{
    int buffer[4];
    void (*function_ptr)(void);
};

int main(void)
{
    struct Challenge challenge;
    int index;
    unsigned int value;

    challenge.function_ptr = normal;

    challenge.buffer[0] = 1;
    challenge.buffer[1] = 2;
    challenge.buffer[2] = 3;
    challenge.buffer[3] = 4;

    printf("buffer:        %p\n", (void *)challenge.buffer);
    printf("function_ptr:  %p\n", (void *)&challenge.function_ptr);
    printf("normal:        %p\n", (void *)normal);
    printf("win:           %p\n", (void *)win);

    printf("\nIndex: ");
    scanf("%d", &index);

    printf("Value (hex): ");
    scanf("%x", &value);

    /*
     * Intentionally vulnerable:
     * index is not checked before accessing buffer.
     */
    challenge.buffer[index] = value;

    printf("\nCalling function pointer...\n");

    challenge.function_ptr();

    return 0;
}