#include <stdio.h>

int main(void)
{
    int buffer[4];
    int secret = 0x12345678;

    printf("buffer: %p\n", (void *)buffer);
    printf("secret: 0x%x\n", secret);

    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 3;
    buffer[3] = 4;

    buffer[4] = 0x41414141;

    printf("buffer[0]: %d\n", buffer[0]);
    printf("buffer[1]: %d\n", buffer[1]);
    printf("buffer[2]: %d\n", buffer[2]);
    printf("buffer[3]: %d\n", buffer[3]);

    return 0;
}