#ifndef AMS_MANAGE_H
#define AMS_MANAGE_H


#define MANAGEOPT   (6)

#define NUMBERLEN   (8)
#define COMMONLEN   (32)

#define REMARKLEN   (256)
#define COMMENTLEN  (256)

#define ATTENDLEN   (1024)
#define BODYLEN     (1024)

#define EVENTLEN    (1048576)

#define EVNDB       "evn.info"

#define COMMENT_SEPARATOR   '^'
#define ATTEND_SEPARATOR    ','

typedef struct ams_event_s {

    char flag[8];
    int num;

    char date[32];
    char time[32];

    char head[32];
    char user[32];

    char remark[256];
    char attend[1024];
    char body[1024];

    char comment[8192];                 // potentially lead to overflow
    struct ams_event_s *next;
} ams_event_t;


AMS_STAT ams_manage(ams_info_t *info);


#endif // AMS_MANAGE_H
