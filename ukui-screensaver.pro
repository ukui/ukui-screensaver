#-------------------------------------------------
#
# Project created by QtCreator 2018-04-09T11:40:40
#
#-------------------------------------------------

TRANSLATIONS = i18n_ts/zh_CN.ts
QT       += core gui dbus x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(src/bioAuthentication/bioAuthentication.pri)

TARGET = ukui-screensaver
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
           QT_MESSAGELOGCONTEXT

LIBS += -lpam -lpam_misc

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 x11 xcb

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/unixsignallistener.cpp \
    src/pam.cpp \
    src/gsettings.cpp \
    src/auxiliary.cpp \
    src/configuration.cpp \
    src/screensaverwidget.cpp \
    src/screensaver.cpp

HEADERS += \
    src/mainwindow.h \
    src/unixsignallistener.h \
    src/pam.h \
    src/gsettings.h \
    src/auxiliary.h \
    src/configuration.h \
    src/screensaverwidget.h \
    src/screensaver.h

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    src/image.qrc
