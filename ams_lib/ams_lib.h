#ifndef AMS_LIB_H
#define AMS_LIB_H

#define FLAGLEN     (8)
#define ams_memset(buf, size) (void)memset(buf, 0, size)
#define ams_free free


void *ams_alloc(size_t size);
void *ams_calloc(size_t size);

AMS_STAT ams_destroyinfo(ams_info_t *p);
AMS_STAT ams_getflag(char *flag);

#endif // AMS_LIB_H
