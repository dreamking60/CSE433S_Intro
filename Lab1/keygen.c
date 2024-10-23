#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define KEYSIZE 16
void main()
{
    
    char key[KEYSIZE];
    for(time_t i = 1524017329; i <= 1524024529; i++) {
        srand(i);
        for (int j = 0; j< KEYSIZE; j++){
            key[j] = rand()%256;
            printf("%.2x", (unsigned char)key[j]);
        }
        printf("\n");
    }
}