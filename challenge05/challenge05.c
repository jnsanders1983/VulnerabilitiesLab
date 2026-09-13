#include <stdio.h>
#include <stddef.h>

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
    size_t buffer_count;

    challenge.function_ptr = normal;

    challenge.buffer[0] = 1;
    challenge.buffer[1] = 2;
    challenge.buffer[2] = 3;
    challenge.buffer[3] = 4;

    buffer_count =
        sizeof(challenge.buffer) / sizeof(challenge.buffer[0]);

    printf("buffer:        %p\n", (void *)challenge.buffer);
    printf("function_ptr:  %p\n", (void *)&challenge.function_ptr);
    printf("normal:        %p\n", (void *)normal);
    printf("win:           %p\n", (void *)win);
    printf("buffer_count:  %zu\n", buffer_count);

    printf("\nIndex: ");

    if (scanf("%d", &index) != 1)
    {
        printf("ERROR: invalid index input.\n");
        return 1;
    }

    printf("Value (hex): ");

    if (scanf("%x", &value) != 1)
    {
        printf("ERROR: invalid value input.\n");
        return 1;
    }

    /*
     * Validate the index before using it.
     *
     * index is signed, while buffer_count is size_t (unsigned).
     * Check for a negative value first, then convert index to
     * size_t for the upper-bound comparison.
     */
    if (index < 0 || (size_t)index >= buffer_count)
    {
        printf("ERROR: index is outside the bounds of buffer.\n");
        return 1;
    }

    challenge.buffer[index] = value;

    printf("\nCalling function pointer...\n");

    challenge.function_ptr();

    return 0;
}