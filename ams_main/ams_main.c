#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <unistd.h>

#include "ams_main.h"
#include "../ams_lib/ams_lib.h"
#include "../ams_login/ams_login.h"
#include "../ams_manage/ams_manage.h"

static AMS_STAT ams_main_prompt(ams_info_t *info)
{
    u_char *ret = &(info->n);
    u_char i = 0;

    printf("\t\t---****----****----****----****----****---\n\n");
    printf("\t\t| Welcome to Activity Management System: |\n\n");

    printf("\t\t*\t\t(1) sign in\t\t *\n\n");
    printf("\t\t*\t\t(2) sign up\t\t *\n\n");
    printf("\t\t|\t\t(3) quit\t\t |\n\n");

    printf("\t\t---****----****----****----****----****---\n");

    printf("\nPlease choose a number and type ENTER\n");

    for(i = 0; i < MAXTRY; i++){
        if((fgets(info->temp, TEMPLEN, stdin)) == NULL) {
            printf("Invalid input, exit");
            return AMS_FAIL;
        }

        *ret = atoi(info->temp);


        if(*ret > MAINOPT || *ret < MINOPT){
            printf("Invalid input, try again please\n");
            ams_memset(info->temp, TEMPLEN);
        } else break;
    }

    if(i >= MAXTRY) {
        printf("Main: 3 times failure, exit\n");
        return AMS_FAIL;
    }

    ams_memset(info->temp, TEMPLEN);

    return AMS_OK;
}


static AMS_STAT ams_loaduser(ams_header_t *hd)
{
    FILE *fp;

    ams_user_t *user = NULL;


    if((user = ams_calloc(sizeof(ams_user_t))) == NULL) return AMS_FAIL;

    if((fp = fopen(USRDB, "a+")) == NULL) {             // "w+" cause a bug: clear info.db after reading it
        printf("Main: failed to open user data: info.db\n");
        return AMS_FAIL;
    }

    while(fscanf(fp, "%s\t%s", user->name, user->passwd) != EOF) {
#if AMS_DEBUG
        printf("Load user: --%s---%s---\n", user->name, user->passwd);
#endif

        if(ams_insertuser(hd, user) == AMS_FAIL) {
            printf("Main: failed to insert an user node\n");
            return AMS_FAIL;
        }

        if((user = ams_calloc(sizeof(ams_user_t))) == NULL) return AMS_FAIL;

        ams_memset(user->name, NAMELEN);
        ams_memset(user->passwd, PASSWDLEN);
    }

    fclose(fp);
    return AMS_OK;
}


static AMS_STAT ams_destroyusrhd(ams_header_t *hd)
{
    ams_user_t *p = NULL;

    if((p = hd->next) == NULL) {
        hd->size = 0;
        ams_free(hd);
        return AMS_OK;
    }

    while(p != NULL) {
#if AMS_DEBUG
        printf("Destroy user: --%s---%s---\n", p->name, p->passwd);
#endif

        ams_memset(p->name, NAMELEN);
        ams_memset(p->passwd, PASSWDLEN);

        if(p->next != NULL) hd->next = p->next;         // p->next not null in default ?!
        else hd->next = NULL;

        ams_free(p);
        p = hd->next;
    }

    hd->size = 0;
    ams_free(hd);

    return AMS_OK;
}


AMS_STAT ams_insertuser(ams_header_t *hd, ams_user_t *user)
{
    ams_user_t *p = NULL;

    if(hd->next == NULL) {

#if AMS_DEBUG
        printf("Insert user: -----header null-----\n");
#endif
        hd->next = user;
        hd->size++;
        user->next = NULL;

    } else {

#if AMS_DEBUG
        printf("Insert user: -----header not null---name: %s----passwd: %s-----\n", user->name, user->passwd);
#endif
        p = hd->next;
        while(p->next != NULL) p = p->next;

        p->next = user;
        hd->size++;
        user->next = NULL;
    }

    return AMS_OK;
}


AMS_STAT ams_saveuser(ams_header_t *hd)
{
    FILE *fp;
    ams_user_t *p = NULL;

    if((p = hd->next) == NULL) {
        printf("Save user: no user to save\n");
        return AMS_FAIL;
    }

    if((fp = fopen(USRDB, "w")) == NULL) return AMS_FAIL;

    while(p != NULL ) {

#if AMS_DEBUG
        printf("Save user: --%s---%s---\n", p->name, p->passwd);
#endif

        if(fprintf(fp, "%s\t%s\n", p->name, p->passwd) <= AMS_FAIL) {
            printf("Save user: failed to save user data\n");
            return AMS_FAIL;
        }
        p = p->next;
    }

    fclose(fp);
    return AMS_OK;
}


AMS_STAT main(/*int argc, char *argv[]*/)
{
    ams_info_t *info = NULL;

    ams_header_t *usrhd = NULL;

    char flag[8] = {'\0'};
    flag[0] = 'Y';

#if !AMS_DEBUG
    system("cls");
#endif
    if((info = ams_calloc(sizeof(ams_info_t))) == NULL) return AMS_FAIL;

    do {
        if((usrhd = ams_calloc(sizeof(ams_header_t))) == NULL) return AMS_FAIL;

        if(ams_loaduser(usrhd) == AMS_FAIL) return AMS_FAIL;

        if(ams_main_prompt(info) == AMS_FAIL) return AMS_FAIL;

        if(info->n == 1)
            if(ams_signin(info, usrhd) == AMS_FAIL) printf("Main: error after signing in\n");

        if(info->n == 2)
            if(ams_signup(info, usrhd) == AMS_FAIL) printf("Main: error after signing up\n");

        if(info->n == 3) { printf("Activity Management System: quit...\n"); goto MAIN;}

        ams_memset(info, sizeof(ams_info_t));

        if(ams_destroyusrhd(usrhd) == AMS_FAIL) return AMS_FAIL;

        printf("Main: return to previous menu, Y/y or N/n?\n");

        if(ams_getflag(flag) == AMS_FAIL) return AMS_FAIL;
#if AMS_DEBUG
        printf("Main: flag[0]---%c----\n", flag[0]);
#endif

        //if( (flag[0] = 'N') || (flag[0] == 'n') ) goto MAIN;

    } while( (flag[0] == 'Y') || (flag[0] == 'y') );      // mistake "=" as  comparison sympol "==" again

MAIN:
    if(ams_destroyinfo(info) == AMS_FAIL) return AMS_FAIL;

    return AMS_OK;
}
