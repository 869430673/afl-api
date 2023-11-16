#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apr_general.h"
#include "apr_errno.h"
#include "apr_file_io.h"

#define SIZE 10240

int main(int argc, char **argv) {

    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);

    apr_int32_t flags = APR_FILEPATH_TRUENAME | APR_FILEPATH_SECUREROOT;

/* Read and data from afl's input */

    FILE *fp = fopen(argv[1], "r");
    if(!fp) {
    printf("Failed to open input file\n");
    return -1;
    }

/* Init variables */
    
    const char* val_1 = NULL;
    val_1 = malloc(sizeof(char) * SIZE);

    const char* val_2 = NULL;
    val_2 = malloc(sizeof(char) * SIZE);

    char *newpath = NULL;

/* Parse data */

    char buffer[SIZE];
    memset(buffer, 0, SIZE);
    char* left = 0, *right;
	
    fgets(buffer, sizeof(buffer) - 1, fp);
    left = strchr(buffer, '=');
    if(left) {
	right = strchr(left + 1, '=');
	if(right) {
	    sscanf(right + 1, "%s", val_1);
	    memset(buffer, 0, SIZE);
        }
    }
	
    fgets(buffer, sizeof(buffer) - 1, fp);
    left = strchr(buffer, '=');
    if(left) {
	right = strchr(left + 1, '=');
	if(right) {
	    sscanf(right + 1, "%s", val_2);
	    memset(buffer, 0, SIZE);
        }
    }

    fclose(fp);

/* Call API */
    printf("%s\n%s\n", val_1, val_2);
    apr_status_t status = apr_filepath_merge(&newpath, val_1, val_2, flags, pool);
    // printf("%s\n%s\n", val_1, val_2);

    if (status == APR_SUCCESS) {
        printf("Merged path: %s\n", newpath);
    } else {
        char errmsg[256];
        apr_strerror(status, errmsg, sizeof(errmsg));
        printf("Failed to merge path: %s\n", errmsg);
    }

    apr_pool_destroy(pool);
    apr_terminate();
    
    return 0;
}
/**
apr_status_t apr_filepath_merge(char **newpath, const char *rootpath, const char *addpath, apr_int32_t flags, apr_pool_t *pool);

**/
