#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define KEYSIZE 16
void main()
{
    
    char key[KEYSIZE];
    time_t start = 1524017329;
    time_t end = 1524024529;
    for(time_t i = start; i <= end; i++) {
        srand(i);
        for (int j = 0; j< KEYSIZE; j++){
            key[j] = rand()%256;
            printf("%.2x", (unsigned char)key[j]);
        }
        printf("\n");
    }
}