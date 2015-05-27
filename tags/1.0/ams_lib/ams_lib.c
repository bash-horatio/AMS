#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../ams_main/ams_main.h"
#include "ams_lib.h"

void *ams_alloc(size_t size)
{
    void *p = NULL;
    p = malloc(size);

    return p;
}

void *ams_calloc(size_t size)
{
    void *p = NULL;

    p = ams_alloc(size);
    if(p) ams_memset(p, size);

    return p;
}


AMS_STAT ams_destroyinfo(ams_info_t *p)
{
    if(p == NULL) return AMS_OK;

    ams_memset(p, sizeof(ams_info_t));
    ams_free(p);

    return AMS_OK;
}


AMS_STAT ams_getflag(char *flag)
{
    int i = 0;

    ams_memset(flag, FLAGLEN);

    for(flag[0] = 'Y'; i < MAXTRY; i++) {

        if(fgets(flag, FLAGLEN - CHARLEN, stdin) == NULL) {
            printf("Manage: flag too long, try again.\n");
            ams_memset(flag, FLAGLEN);
            continue;
        }

        flag[1] = '\0';

        if( (flag[0] != 'Y' ) && (flag[0] != 'y')
                && (flag[0] != 'N') && (flag[0] != 'n') ) {

            printf("Manage: invalid flag, try again.\n");
            ams_memset(flag, FLAGLEN);

        } else break;
    }

    if(i >= MAXTRY) {
        printf("Manage: 3 times failure, exit\n");
        return AMS_FAIL;
    }

    return AMS_OK;
}



