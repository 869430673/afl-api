#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

/* Include files */
#include "nanoarrow.h"
/* End */

#define ARG_NUM 1
#define SIZE 10240

int main(int argc, char **argv) {

    /* Read and data from afl's input */

    FILE* fp = fopen(argv[1], "r");
    if(!fp) {
        printf("Failed to open input file\n");
        exit(-1);
    }

    /* Init variables */
    const char* val_0 = (const char* )malloc(SIZE);
    char* val_1 = (char* )malloc(SIZE);
    /* End Init */

    /* Parse data */
    char buffer[SIZE];
    memset(buffer, 0, SIZE);
    char* left = 0, *right = 0;

    fgets(buffer, sizeof(buffer) - 1, fp);
    left = strchr(buffer, '=');
    if(left) {
        right = strchr(left + 1, '=');
        if(right) {
            sscanf(right + 1, "%s", (char*)val_0);
            memset(buffer, 0, SIZE);
        }
    }

    fgets(buffer, sizeof(buffer) - 1, fp);
    left = strchr(buffer, '=');
    if(left) {
        right = strchr(left + 1, '=');
        if(right) {
            sscanf(right + 1, "%s", (char*)val_1);
            memset(buffer, 0, SIZE);
        }
    }
    /* End Parse */

    fclose(fp);
    /* End */

    /* Call API */
    int8_t ret_0 = _ArrowParseUnionTypeIds((const char*)val_0, (int8_t*)val_1);

    /* Well Done! */
    return 0;
}
