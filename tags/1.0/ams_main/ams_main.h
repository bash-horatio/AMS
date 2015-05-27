#ifndef AMS_MAIN_H
#define AMS_MAIN_H


#define AMS_DEBUG   (0)

#define AMS_STAT    int
#define AMS_OK      (0)
#define AMS_FAIL    (-1)

#define TEMPLEN    (1024)
#define CHARLEN    (1)

#define NAMELEN    (32)
#define PASSWDLEN  (32)

#define MAXTRY      (3)
#define MAINOPT     (3)
#define MINOPT      (1)

#define USRDB       "usr.info"

typedef struct ams_header_s {

    void *next;
    size_t size;
} ams_header_t;

typedef struct ams_user_s {

    char name[32];
    char passwd[32];
    struct ams_user_s *next;
} ams_user_t;

typedef struct ams_info_s {

    unsigned char n;
    unsigned char flag;

    char name[32];
    char passwd[32];

    FILE *db;
    char temp[1024];
} ams_info_t ;


AMS_STAT ams_insertuser(ams_header_t *hd, ams_user_t *user);
AMS_STAT ams_saveuser(ams_header_t *hd);

#endif // AMS_MAIN_H
