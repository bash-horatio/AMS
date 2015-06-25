TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ams_main/ams_main.c \
    ams_lib/ams_lib.c \
    ams_login/ams_login.c \
    ams_manage/ams_manage.c

HEADERS += \
    ams_main/ams_main.h \
    ams_lib/ams_lib.h \
    ams_login/ams_login.h \
    ams_manage/ams_manage.h

