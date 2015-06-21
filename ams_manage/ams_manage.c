#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <sys/stat.h>

#include "../ams_main/ams_main.h"
#include "../ams_lib/ams_lib.h"
#include "../ams_login/ams_login.h"
#include "ams_manage.h"


static AMS_STAT ams_manage_prompt(ams_info_t *info)
{
    u_char *ret = &(info->n);
    u_char i = 0;

    info->n = 0;

    printf("\t\t---****----****----****----****----****---\n\n");
    printf("\t\t| Welcome to Activity Management System: |\n\n");

    printf("\t\t*\t\t(1) create\t\t *\n\n");
    printf("\t\t|\t\t(2) delete\t\t |\n\n");

    printf("\t\t*\t\t(3) browse \t\t *\n\n");
    printf("\t\t|\t\t(4) comment\t\t |\n\n");

    printf("\t\t*\t\t(5) attend\t\t *\n\n");
    printf("\t\t|\t\t(6) quit\t\t |\n\n");

    printf("\t\t---****----****----****----****----****---\n");

    printf("Please choose a number and type ENTER\n");

    for(i = 0; i < MAXTRY; i++){
        if((fgets(info->temp, TEMPLEN, stdin)) == NULL) {
            printf("Manage: invalid input, exit\n");
            return AMS_FAIL;
        }

        *ret = atoi(info->temp);

        if(*ret > MANAGEOPT || *ret < MINOPT){
            printf("Manage: no this choice, try again please\n");
            ams_memset(info->temp, TEMPLEN);
        } else break;
    }

    if(i >= MAXTRY) {
        printf("Manage: 3 times failure, exit\n");
        return AMS_FAIL;
    }

    ams_memset(info->temp, TEMPLEN);

    return AMS_OK;
}


static AMS_STAT ams_event_prompt(char *ch, size_t len, char *prompt)
{
    u_char i = 0;

    printf("%s\n", prompt);
    for(i = 0; i < MAXTRY; i++) {

        if(fgets(ch , len, stdin) == NULL) {
            printf("Manage: content too long, try again\n");
            ams_memset(ch, len);                            // avoid unexpected corruption actively
        } else break;

    }

    ch[strlen(ch) - CHARLEN] = '\0';

    if(i >= MAXTRY) {
        printf("Manage: 3 times failure, exit\n");
        return AMS_FAIL;
    }

    return AMS_OK;
}


static AMS_STAT ams_save_event(ams_info_t *info, ams_header_t *hd)
{
    FILE *fp;
    ams_event_t *evn = NULL;

    if((evn = hd->next) == NULL && info->n != 2 ) {
        printf("Save event: no created event to save, error\n");
        return AMS_FAIL;
    }

    if((fp = fopen(EVNDB, "w")) == NULL) return AMS_FAIL;

    while(evn != NULL ) {

#if AMS_DEBUG
        printf("Save event: --%s--%s--%d--%s--%s--%s--%s--%s--%s--\n", evn->head, evn->user,
               evn->num, evn->date, evn->time, evn->body, evn->remark, evn->attend,
               evn->comment);
#endif

        if(fprintf(fp, "%s\n%s\n%d\n%s\n%s\n%s\n%s\n%s\n%s\n\n", evn->head, evn->user,      // COMMENT_SEPARATOR EOL for comment
                   evn->num,evn->date, evn->time, evn->body, evn->remark,
                   evn->attend, evn->comment) == AMS_FAIL) return AMS_FAIL;

        evn = evn->next;
    }

    fclose(fp);

    return AMS_OK;
}


static AMS_STAT ams_insert_event(ams_header_t *hd, ams_event_t *evn)
{
    evn->flag[0] = 'Y';
    ams_event_t *p = NULL;

#if AMS_DEBUG
    printf("Insert: --%s--%s--%d--%s--%s--%s--%s--%s--%s--\n", evn->head, evn->user,
           evn->num, evn->date, evn->time, evn->body, evn->remark, evn->attend, evn->comment);
#endif

    if(hd->next == NULL) {
        hd->next = evn;
        hd->size++;
        evn->next = NULL;

    } else {
        p = hd->next;
        while(p->next != NULL) p = p->next;

        p->next = evn;
        evn->next = NULL;
    }

    return AMS_OK;
}


static AMS_STAT ams_read_eventitem(char *item, int len, char **p)
{
    int i = 0;
    ams_memset(item, len);
    char *ch = *p;


    for(i = 0; (i < len) && (*ch != '\n'); i++, ch++) item[i] = *ch;

#if AMS_DEBUG
    printf("Read item---%s---\n", item);
#endif

    (*p) += strlen(item) + CHARLEN;

    return AMS_OK;
}


static AMS_STAT ams_load_event(ams_header_t *hd)
{
    FILE *fp;
    char *ch = NULL;
    char num[8] = {'\0'};
    char temp[EVENTLEN];

    ams_memset(temp, EVENTLEN);
    ams_event_t *evn = NULL;

    if((fp = fopen(EVNDB, "a+")) == NULL) {
        printf("Manage: failed to open event data\n");
        return AMS_FAIL;
    }

    struct stat state = {0};
    if(stat(EVNDB, &state) == AMS_FAIL) return AMS_FAIL;

    fread(temp, 1, state.st_size, fp);
    ch = temp;

#if AMS_DEBUG
    printf("Load event: stat: file size is %d\n", (u_int )(state.st_size));
    printf("Load event: temp-----%s-------\n", temp);
#endif

    if(ferror(fp) != AMS_OK) {
        printf("Manage: failed to read event data\n");
        return AMS_FAIL;
    }

    while(*ch != '\n' && *(ch + 1) != '\0') {

        if((evn = ams_calloc(sizeof(ams_event_t))) == NULL) return AMS_FAIL;

        if(ams_read_eventitem(evn->head, COMMONLEN, &ch) == AMS_FAIL) return AMS_FAIL;

        if(ams_read_eventitem(evn->user, COMMONLEN, &ch) == AMS_FAIL) return AMS_FAIL;

        if(ams_read_eventitem(num, NUMBERLEN, &ch) == AMS_FAIL) return AMS_FAIL;
        evn->num = atoi(num);  // unreasonable way to get

        if(ams_read_eventitem(evn->date, COMMONLEN, &ch) == AMS_FAIL) return AMS_FAIL;
        if(ams_read_eventitem(evn->time, COMMONLEN, &ch) == AMS_FAIL) return AMS_FAIL;

        if(ams_read_eventitem(evn->body, BODYLEN, &ch) == AMS_FAIL) return AMS_FAIL;
        if(ams_read_eventitem(evn->remark, REMARKLEN, &ch) == AMS_FAIL) return AMS_FAIL;

        if(ams_read_eventitem(evn->attend, ATTENDLEN, &ch) == AMS_FAIL) return AMS_FAIL;
        //ch++;
        if(ams_read_eventitem(evn->comment, COMMENTLEN, &ch) == AMS_FAIL) return AMS_FAIL;

#if AMS_DEBUG
        printf("Load Event: --%s--%s--%d--%s--%s--%s--%s--remark\n", evn->head, evn->user,
               evn->num, evn->date, evn->time, evn->body, evn->remark);
        printf("Load Event: -------------attend---%s----------\n", evn->attend);
        printf("Load Event: -------------comment---%s----------\n", evn->comment);
#endif

        if(ams_insert_event(hd, evn) == AMS_FAIL) {

            printf("Manage: failed to insert an event node\n");
            return AMS_FAIL;
        }
        ch++;           // skip '\n' between events on database file (from save event);
    }

    fclose(fp);
    return AMS_OK;
}


static AMS_STAT ams_checkattend(ams_info_t *info, char *ch)
{
    while(*ch != ATTEND_SEPARATOR) {

        if(strncmp(ch, info->name, strlen(info->name)) == AMS_OK) {
            if(strcpy(info->temp, ch) == NULL) return AMS_FAIL;
            return AMS_OK;
        }

        while(*ch != ATTEND_SEPARATOR) ch++;
        ch++;
    }

    return AMS_FAIL;
}


static AMS_STAT ams_get_event(ams_event_t *temp, ams_info_t *info, ams_header_t *hd)
{
    ams_event_t *evn = NULL;

    temp->flag[0] = 'Y';
    temp->attend[0] = ATTEND_SEPARATOR;
    temp->comment[0] = COMMENT_SEPARATOR;

    if(strncpy(temp->user, info->name, strlen(info->name)) == NULL) {
        printf("Create: failed to get the event creator\n");
        return AMS_FAIL;
    }

    if(ams_event_prompt(temp->head, COMMONLEN, "type the event head:") == AMS_FAIL) {
        printf("Create: failed to get the head\n");
        return AMS_FAIL;
    }

    if((evn = hd->next) != NULL) {                               // avoid an user to create the same event

        if(strcmp(evn->head, temp->head) == AMS_OK && strcmp(evn->user, temp->user) == AMS_OK) {
            printf("Create: you already created this event, quit\n");
            ams_memset(temp->user, NAMELEN);
            ams_memset(temp->head, COMMENTLEN);
            return AMS_OK;
        }

        evn = evn->next;
    }

    if(ams_event_prompt(temp->date, COMMONLEN, "type the event date:") == AMS_FAIL) {
        printf("Create: failed to get the date\n");
        return AMS_FAIL;
    }

    if(ams_event_prompt(temp->time, COMMONLEN, "type the event time:") == AMS_FAIL) {
        printf("Create: failed to get the event time\n");
        return AMS_FAIL;
    }

    if(ams_event_prompt(temp->body, BODYLEN, "type the body:") == AMS_FAIL) {
        printf("Create: failed to get the body\n");
        return AMS_FAIL;
    }

    if(ams_event_prompt(temp->remark, REMARKLEN, "type the remark:") == AMS_FAIL) {
        printf("Create: failed to get the remarks\n");
        return AMS_FAIL;
    }

    temp->num = 0;

    return AMS_OK;
}


static AMS_STAT ams_create_event(ams_info_t *info, ams_header_t *hd)
{
    ams_event_t *evn = NULL;

#if !AMS_DEBUG
    system("cls");
#endif

    do {
        evn = ams_calloc(sizeof(ams_event_t));

        if(ams_get_event(evn, info, hd) == AMS_FAIL) return AMS_FAIL;

        if(ams_insert_event(hd, evn) == AMS_OK) printf("Create: success to create\n");
        else printf("Create: failed to create\n");

        printf("Create: save the event, Y/y or N/n?\t");

        if(ams_getflag(evn->flag) == AMS_FAIL) return AMS_FAIL;

        if(evn->flag[0] == 'Y' || evn->flag[0] == 'y') {

            if(ams_save_event(info, hd) == AMS_FAIL) {
                printf("Create: failed to save event\n");
                return AMS_FAIL;
            }

        } else {
            printf("Create: abort the created event\n");
            return AMS_FAIL;
        }

        printf("Create: continue to create, Y/y or N/n?\t");
        if(ams_getflag(evn->flag) == AMS_FAIL) return AMS_FAIL;

    } while( (evn->flag[0] == 'Y') || (evn->flag[0] == 'y') );

    return AMS_OK;
}


static AMS_STAT ams_delete_event(ams_info_t *info, ams_header_t *hd)
{
    ams_event_t *evn = NULL;
    ams_event_t *p = NULL;

    u_char i = 0;
    char head[32] = {'\0'};

    char flag[8] = {'\0'};
    flag[0] = 'N';

#if !AMS_DEBUG
    system("cls");
#endif

    if((evn = hd->next) == NULL) {
        printf("Delete: no event data, create one first\n");
        return AMS_FAIL;
    }

    for(i = 0; i < MAXTRY; i++) {

        ams_memset(head, COMMONLEN);

        if(ams_event_prompt(head, COMMONLEN, "Delete: type the event head:") == AMS_FAIL) {
            printf("Delete: failed to get the head\n");

            continue;
        }

        while(evn != NULL) {

            if(strcmp(evn->head, head) != AMS_OK) {
                p = evn;
                evn = evn->next;
                continue;
            }

            if(strcmp(evn->user, info->name) != AMS_OK) {
                printf("Delete: cannot delete other's event\n");
                info->n = 0;                                                        // affect main menu option
                return AMS_FAIL;
            }

#if AMS_DEBUG
            printf("Delete event: ----%s-----\n", head);
#endif
            if(p == NULL && evn->next == NULL && hd->next == evn)  {                // bug if first node matched

                printf("Delete: no events broadcast on system after this action\n");
                hd->next = NULL;
            }

            else if(p != NULL && evn->next != NULL) p->next = evn->next;            // only four conditions
            else if(p == NULL && evn->next != NULL) hd->next = evn->next;
            else p->next = NULL;

            evn->next = NULL;
            ams_memset(evn, sizeof(ams_event_t));

            ams_free(evn);
            goto DELETE_EVENT;
        }
    }

    printf("Delete: event not exits, try again\n");
    evn = hd->next;


    if(i >= MAXTRY) {
        printf("Delete: 3 times failure, exit\n");
        return AMS_FAIL;
    }

DELETE_EVENT:
    printf("Delete: confirm to delete, Y/y or N/n?\n");

    if(ams_getflag(flag) == AMS_FAIL) return AMS_FAIL;

    if( (flag[0] == 'Y') || (flag[0] == 'y') ) {

        if(ams_save_event(info, hd) == AMS_FAIL) {
            printf("Delete: failed to save the deletion\n");
            info->n = 0;
            return AMS_FAIL;
        }

        printf("Delete: success to delete the event\n");

    } else printf("Delete: abort\n");

    return AMS_OK;
}


static AMS_STAT ams_browse_event(ams_info_t *info, ams_header_t *hd)
{   
    info = info;

    ams_event_t *evn = NULL;
    u_char i = 0;

    char flag[8] = {'\0'};
    char *ch = NULL;

    char attend[NAMELEN];
    ams_memset(attend, NAMELEN);

    char comment[COMMENTLEN];
    ams_memset(comment, COMMENTLEN);

#if !AMS_DEBUG
    system("cls");
#endif
    
    if((evn = hd->next) == NULL) {
        printf("Browse: no event data, create one first\n");
        info->n = 0;
        return AMS_FAIL;
    }
    
    while(evn != NULL) {
        
        printf("\t\t ----****---- Browse events ----****----\n\n");
        
        printf("Head: %s\n", evn->head);
        printf("Creator: %s\t\t\t", evn->user);
        
        printf("Date: %s\t\t\t", evn->date);
        printf("Time: %s\n", evn->time);
        
        printf("Body: %s\n", evn->body);
        printf("Remarks: %s\n\n", evn->remark);

        printf("Attendance Number: %d\n", evn->num);

        ch = evn->attend;
        printf("Attedances:\n");

        while(*ch != ATTEND_SEPARATOR) {                                         // "," EOL from ams_save_event
            while((*ch != ATTEND_SEPARATOR) && (i < (COMMENTLEN - CHARLEN))) {

                attend[i] = *ch;
                ch++;
                i++;
            }

            printf("%s\n", attend);
            ams_memset(attend, NAMELEN);

            i = 0;                                                  // initialize
            ch++;
        }
        printf("\n");

        ch = evn->comment;
        printf("Comments:\n");

        while(*ch != COMMENT_SEPARATOR) {                                         // ";" EOL from ams_save_event
            while((*ch != COMMENT_SEPARATOR) && (i < (COMMENTLEN - CHARLEN))) {

                comment[i] = *ch;
                ch++;
                i++;
            }

            printf("%s\n", comment);
            ams_memset(comment, COMMENTLEN);

            i = 0;                                                  // initialize
            ch++;
        }
        printf("\n");

        printf("\t\t ----****---- Browse events ----****----\n\n");

        flag[0] = 'Y';

        printf("Browse: type Y/y for next event or N/n to quit\n");
        if(ams_getflag(flag) == AMS_FAIL) return AMS_FAIL;

        if( (flag[0] == 'Y') || (flag[0] == 'y') ) {
            if((evn = evn->next) == NULL) printf("Browse: this is the last event\n");
            continue;
        } else break;
    }

    return AMS_OK;
}


static AMS_STAT ams_comment_event(ams_info_t *info, ams_header_t *hd)           // bug: the same user to create the same event
{
    ams_event_t *p = NULL;
    u_char i = 0;

    char temp[COMMENTLEN - NAMELEN] = {'\0'};
    char comment[COMMENTLEN] = {'\0'};

    char head[32] = {'\0'};
    char flag[8] = {'\0'};

#if !AMS_DEBUG
    system("cls");
#endif

    if((p = hd->next) == NULL) {
        printf("Comment: no event exits, create one first\n");
        info->n = 0;
        return AMS_FAIL;
    }

    for(i = 0; i < MAXTRY; i++) {

        if(ams_event_prompt(head, COMMONLEN, "Comment: type the event head:") == AMS_FAIL) {
            printf("Comment: failed to get the event head, try again\n");

            ams_memset(head, COMMENTLEN - NAMELEN);
            continue;
        }

        while(p != NULL) {

            if(strcmp(p->head, head) != AMS_OK) {
                p = p->next;
                continue;
            }

            if(ams_event_prompt(temp, COMMONLEN, "Comment: type a comment:") == AMS_FAIL) {
                printf("Comment: failed to get the comment\n");
                ams_memset(comment, COMMENTLEN - NAMELEN);
                return AMS_FAIL;
            } else goto COMMENT_EVENT;
        }

        printf("Comment: failed to find event head: %s\n", head);
        p = hd->next;
    }

    if(i >= MAXTRY) {
        printf("Comment: 3 times failure, exit\n");
        return AMS_FAIL;
    }

COMMENT_EVENT:

    printf("Comment: save the comment, Y/y or N/n?\n");
    if(ams_getflag(flag) == AMS_FAIL) return AMS_FAIL;

    if(sprintf(comment, "%s: %s^", info->name, temp) == AMS_FAIL && comment == NULL)        // sscanf(char *src, ...)
        return AMS_FAIL;

    if(p->comment[0] == COMMENT_SEPARATOR) {
        if(strncpy(p->comment, comment, strlen(comment)) == NULL)
            return AMS_FAIL;

    } else {
        p->comment[strlen(p->comment) - CHARLEN] = '\0';
        if(strncat(p->comment, comment, strlen(comment)) == NULL) return AMS_FAIL;
    }

    if(strcat(p->comment, "^") == NULL) return AMS_FAIL;

#if AMS_DEBUG
    printf("comment temp------%s-----\n", comment);
    printf("p comment------%s-----\n", p->comment);
#endif

    if( (flag[0] == 'Y') || (flag[0] == 'y') )
        if(ams_save_event(info, hd) == AMS_FAIL) {
            printf("Comment: failed to save the comment\n");
            return AMS_FAIL;
        }

    return AMS_OK;
}


static AMS_STAT ams_attend_event(ams_info_t *info, ams_header_t *hd)
{
    ams_event_t *p = NULL;
    char head[32] = {'\0'};
    char flag[8] = {'\0'};
    char attend[64] = {'\0'};

    u_char i = 0;

#if !AMS_DEBUG
    system("cls");
#endif

    if((p = hd->next) == NULL) {
        printf("Attend: no event exits, create one first\n");
        return AMS_FAIL;
    }

    for(i = 0; i < MAXTRY; i++) {

        if(ams_event_prompt(head, COMMONLEN, "Attend: type the event head:") == AMS_FAIL) {
            printf("Attend: failed to get the event head, try again\n");
            ams_memset(head, COMMENTLEN - NAMELEN);
            continue;
        }

        while(p != NULL) {

            if(strcmp(p->head, head) != AMS_OK) {
                p = p->next;
                continue;
            } else goto ATTEND_EVENT;
        }

        printf("Attend: failed to find event head: %s\n", head);
        p = hd->next;
    }

    if(i >= MAXTRY) {
        printf("Attend: 3 times failure, exit\n");
        return AMS_FAIL;
    }

ATTEND_EVENT:

    printf("Attend: attend this event? Y/y for attend or N/n for cancel the attendance\n");
    if(ams_getflag(flag) == AMS_FAIL) return AMS_FAIL;

    if( (flag[0] == 'Y') || (flag[0] == 'y') ) {

        if(ams_checkattend(info, p->attend) == AMS_OK) {
            printf("Attend: you already signed up to attend\n");
            return AMS_OK;
        }

        p->num++;

        if(sprintf(attend, "%s,", info->name) == AMS_FAIL && attend == NULL)
            return AMS_FAIL;

        if(p->attend[0] == ATTEND_SEPARATOR) {

            if(strcpy(p->attend, attend) == NULL)
                return AMS_FAIL;
        } else {

            p->attend[strlen(p->attend) - CHARLEN] = '\0';
            if(strcat(p->attend, attend) == NULL)
                return AMS_FAIL;
        }
        if(strcat(p->attend, ",") == NULL) return AMS_FAIL;             //EOL for browse to display attendances
    } else {

        if(ams_checkattend(info, p->attend) != AMS_OK) {
            printf("Attend: not signed up to attend, no need to cancel\n");
            return AMS_OK;
        }
#if AMS_DEBUG
        printf("Attend: cancel p.attend before--%s--\n", p->attend);
#endif

        i = strlen(p->attend) - strlen(info->temp);
#if AMS_DEBUG
        p->attend[i] = '\0';
#endif

        printf("Attend: cancel p.attend temp--%s--\n", p->attend);
        i = strlen(info->name) + CHARLEN;                                   // array cannot be "<<"?

        if(strcat(p->attend, &(info->temp[i])) == NULL) return AMS_FAIL;
        p->num--;

#if AMS_DEBUG
        printf("Attend: cancel info.temp--%s--\n", &(info->temp[i]));
        printf("Attend: cancel p.attend cat--%s--\n", p->attend);
#endif
    }

    if(ams_save_event(info, hd) == AMS_FAIL) {
        p->num--;                                                           // remove the unsaved comment
        printf("Attend: fail to sign up to attend\n");
        return AMS_FAIL;
    }


    return AMS_OK;
}


static AMS_STAT ams_destroyevnhd(ams_header_t *hd)
{
    ams_event_t *p = NULL;

    if((p = hd->next) == NULL) {
        hd->size = 0;
        ams_free(hd);
        return AMS_OK;
    }

    while(p != NULL) {
#if AMS_DEBUG
        printf("Destroy event: --%s--%s--%d--%s--%s--%s--%s--%s--%s--\n", p->head, p->user,
               p->num, p->date, p->time, p->body, p->remark, p->attend, p->comment);
#endif

        if(p->next != NULL) hd->next = p->next;                                 // p->next not null in default ?!
        else hd->next = NULL;

        ams_free(p);
        p = hd->next;
    }

    hd->size = 0;
    ams_free(hd);

    return AMS_OK;
}


AMS_STAT ams_manage(ams_info_t *info)
{
    char flag[8] = {'\0'};
    flag[0] = 'N';
    ams_header_t *evnhd = NULL;

#if !AMS_DEBUG
    system("cls");
#endif
    do {
        if((evnhd = ams_calloc(sizeof(ams_header_t))) == NULL) return AMS_FAIL;
        if(ams_load_event(evnhd) == AMS_FAIL) return AMS_FAIL;

        if(ams_manage_prompt(info) == AMS_FAIL) return AMS_FAIL;

        if(info->n == 1) if(ams_create_event(info, evnhd) == AMS_FAIL)
            printf("Manage: create error\n");                                           // unreasonable in a way

        if(info->n == 2) if(ams_delete_event(info, evnhd) == AMS_FAIL)
            printf("Manage: delete error\n");

        if(info->n == 3) if(ams_browse_event(info, evnhd) == AMS_FAIL)
            printf("Manage: browse error\n");

        if(info->n == 4) if(ams_comment_event(info, evnhd) == AMS_FAIL)
            printf("Manage: comment error\n");

        if(info->n == 5) if(ams_attend_event(info, evnhd) == AMS_FAIL)
            printf("Manage: attend error\n");

        if(info->n == 6) { printf("Manage: quit...\n"); break;}

        if(ams_destroyevnhd(evnhd) == AMS_FAIL) {
            printf("Manage: failed to destroy event header\n");
            return AMS_FAIL;
        }

        printf("Manage: return to previous menu, Y/y or N/n?\n");

        if(ams_getflag(flag) == AMS_FAIL) return AMS_FAIL;

#if AMS_DEBUG
        printf("Manage: flag[0]---%c----\n", flag[0]);
#endif

        //if( (flag[0] = 'N') || (flag[0] == 'n') ) return AMS_OK;          // bug

    } while( (flag[0] == 'Y') || (flag[0] == 'y'));

    return AMS_OK;
}
