#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>                            // windows only
#include <conio.h>                              // windows only

#include "../ams_main/ams_main.h"
#include "../ams_lib/ams_lib.h"
#include "../ams_login/ams_login.h"
#include "../ams_manage/ams_manage.h"


static AMS_STAT ams_getpasswd(ams_info_t *info)
{
    u_char i = 0;

    info->n = 0;
    ams_memset(info->temp, TEMPLEN);
    ams_memset(info->passwd, PASSWDLEN);

    while( (char )(info->n = getch()) != '\r') {                // '\r' stand for ENTER on windows

        if ( (i == PASSWDLEN - CHARLEN) && info->n != '\b') {
            printf("Sign in: Max length of password is 31\n");
            ams_memset(info->passwd, PASSWDLEN + CHARLEN);
            break;
        }


        if(info->n == '\b') {

            if(i == 0) continue;
            else {
                printf(PASSWDBACK);
                i--;
            }
        } else if( (char )(info->n) != '\r') {

            putchar(PASSWDMASK);                        // how to mask it after 1 second delay
            (info->passwd)[i] = info->n;

            i++;
        }

    }

    printf("\n");
    //    printf("Get password----%s---\n", info->passwd);

    return AMS_OK;
}


static AMS_STAT ams_verifyuser(ams_info_t *info, char *name, char *tag)
{
    char *prompt = NULL;

    if(info->flag == 1) prompt = "in";
    else if(info->flag == 2) prompt = "up";

    else {
        printf("Verify user: invalid flag\n");
        return AMS_FAIL;
    }

    if(tag != NULL) goto USR_L;

    printf("Sign %s: type an user name:\n", prompt);

    ams_memset(info->name, NAMELEN);                    // avoid to affect regain the user name

    if((fgets(info->name, NAMELEN, stdin)) == NULL) {
        printf("Sign %s: user name too long, try again\n", prompt);
        return AMS_FAIL;
    }

    info->name[strlen(info->name) - CHARLEN] = '\0';

USR_L:

#if AMS_DEBUG
    fprintf(stdout, "name----%s----%d------\n", name, strlen(name));
    printf("info->name----%s----%d--\n", info->name, strlen(info->name));
#endif

    if(strncmp(info->name, name, strlen(name)) != AMS_OK) return AMS_FAIL;

    return AMS_OK;
}


static AMS_STAT ams_verifypasswd(ams_info_t *info, char *passwd)
{
    char *prompt = NULL;
    char again[8] = {'\0'};

    if(info->flag == 1) prompt = "in";
    else if(info->flag == 2) {
        prompt = "up";
        strcpy(again, " again");
    }

    else {
        printf("Verify password: invalid flag\n");
        return AMS_FAIL;
    }

    printf("Sign %s: type your password \b%s:\n", prompt, again);

    if(ams_getpasswd(info) == AMS_FAIL) {
        printf("Sign %s: fail to get user's password\n", prompt);
        ams_memset(info->name, NAMELEN);
        return AMS_FAIL;
    }

#if AMS_DEBUG
    fprintf(stdout, "Verify passwd: passwd----%s----%d------\n", passwd, strlen(passwd));
    printf("Verify passwd: info->passwd----%s----%d--\n", info->passwd, strlen(info->passwd));
#endif

    if(strncmp(info->passwd, passwd, strlen(passwd)) != AMS_OK) return AMS_FAIL;

    return AMS_OK;
}


static AMS_STAT ams_verify(ams_info_t *info, ams_header_t *hd)
{
    u_char i = 0;
    char *tag = NULL;

    ams_user_t *p = NULL;
    if((p = hd->next) == NULL) {
        printf("Sign in: no user exists, sign up one first\n");;
        return AMS_FAIL;
    }

    info->flag = 1;                                     // for verify user and password

    for(i = 0; i < MAXTRY; i++) {

        while( p != NULL ) {

            if(ams_verifyuser(info, p->name, tag) == AMS_FAIL) {

                p = p->next;
                if(p != NULL)  {
                    tag = info->name;
                    continue;
                } else {
                    p = hd->next;
                    printf("Sign in: no such an user name, try again\n");
                    tag = NULL;
                    break;
                }

            }

            if(ams_verifypasswd(info, p->passwd) == AMS_FAIL) {
                printf("Sign in: invalid password, try again\n");
                p = hd->next;
                break;
            } else return AMS_OK;

        }

    }

    if(i >= MAXTRY) printf("Sign in: 3 times failure, exit\n");

    return AMS_FAIL;
}


AMS_STAT ams_signin(ams_info_t *info, ams_header_t *hd)
{
    ams_memset(info->temp, TEMPLEN);
    info->n = 0;

    if(ams_verify(info, hd) == AMS_FAIL) return AMS_FAIL;

    if(ams_manage(info) == AMS_FAIL) return AMS_FAIL;

    info->n = 0;

    return AMS_OK;
}


AMS_STAT ams_signup(ams_info_t *info, ams_header_t *hd)
{
    ams_user_t *p = NULL;
    ams_info_t *temp;

    char *name = NULL;
    u_char i = 0;

    if((temp = ams_calloc(sizeof(ams_info_t))) == NULL) return AMS_FAIL;

    info->flag = 2;                                     // for verify user and password
    if((p = hd->next) == NULL) {

        printf("Sign up: type an user name:\n");

        ams_memset(info->name, NAMELEN);                // problem

        if((fgets(info->name, NAMELEN, stdin)) == NULL) {
            printf("Sign up: user name too long, try again\n");
            return AMS_FAIL;
        }

        info->name[strlen(info->name) - CHARLEN] = '\0';
        goto SIGNUP_VERIFYPASSWD;
    }


    for(i = 0; i < MAXTRY; i++) {
        while(p != NULL) {

            if(ams_verifyuser(info, p->name, name) == AMS_OK) {
                printf("Sign up: user name already exists, try again\n");
                p = hd->next;
                name = NULL;
                break;
            }

            if(p->next != NULL) {
                p = p->next;
                name = info->name;
                continue;
            } else {
                printf("Sign up: luckily, no one has picked up this name\n");
                goto SIGNUP_VERIFYPASSWD;
            }
        }
    }

    if(i >= MAXTRY) {
        if(i >= MAXTRY) printf("Sign in: 3 times failure, exit\n");
        return AMS_FAIL;
    }

SIGNUP_VERIFYPASSWD:

    for(i = 0; i < MAXTRY; i++) {

        printf("Sign up: type your password:\n");

        if(ams_getpasswd(info) == AMS_FAIL) {
            printf("Sign up: failed to get password");
            ams_memset(info->passwd, PASSWDLEN);
            continue;
        }

        temp->flag = 2;

        if(ams_verifypasswd(temp, info->passwd) == AMS_FAIL) {
            printf("Sign up: two passwords not matched, try again\n");
            ams_memset(info->passwd, PASSWDLEN);
            ams_memset(temp->passwd, PASSWDLEN);
        } else {

            if((p = ams_calloc(sizeof(ams_user_t))) == NULL) return AMS_FAIL;

            if(strcpy(p->name, info->name) == NULL) return AMS_FAIL;
            if(strcpy(p->passwd, info->passwd) == NULL) return AMS_FAIL;

            if(ams_insertuser(hd, p) == AMS_FAIL) return AMS_FAIL;

            if(ams_saveuser(hd) ==AMS_FAIL) {
                printf("Save user: failed to open user data: info.db\n");
                return AMS_FAIL;
            }

            printf("Sign up: sign in as \"%s\" now\n", info->name);
            break;
        }
    }

    if(i >= MAXTRY) {
        printf("Sign in: 3 times failure, exit\n");
        return AMS_FAIL;
    }

    info->n = 0;
    fclose(info->db);

    return AMS_OK;
}
