#include <stdio.h>

int main(void)
{
    int buffer[4];
    int target = 0x12345678;

    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 3;
    buffer[3] = 4;

    printf("buffer: %p\n", (void *)buffer);
    printf("target: 0x%x\n", target);

    return 0;
}