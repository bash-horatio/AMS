#ifndef AMS_ACT_H
#define AMS_ACT_H


#define PASSWDBACK "\b \b"
#define PASSWDMASK '*'


AMS_STAT ams_signin(ams_info_t *info, ams_header_t *usrhd);
AMS_STAT ams_signup(ams_info_t *info, ams_header_t *usrhd);

#endif // AMS_ACT_H
