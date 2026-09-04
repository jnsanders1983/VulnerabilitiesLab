#include <stdio.h>

void inspect(int value)
{
    int local = 42;
    int *ptr = &local;

    printf("value: %d\n", value);
    printf("local: %d\n", local);
    printf("ptr:   %p\n", (void *)ptr);
}

int main(void)
{
    int number = 1234;

    printf("number: %d\n", number);
    printf("&number: %p\n", (void *)&number);

    inspect(number);

    return 0;
}