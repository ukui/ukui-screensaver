#-------------------------------------------------
#
# Project created by QtCreator 2018-04-09T11:40:40
#
#-------------------------------------------------
QT       += core gui dbus x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(BioAuth/bioauth.pri)
include(VirtualKeyboard/VirtualKeyboard.pri)

TARGET = ukui-screensaver-dialog
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
           QT_MESSAGELOGCONTEXT

LIBS += -lpam -lpam_misc -lX11 -lXext -lXtst

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 x11 xcb xtst

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    VirtualKeyboard/src/
    BioAuth/include/

SOURCES += \
    ukui-screensaver-dialog.cpp \
    mainwindow.cpp \
    unixsignallistener.cpp \
    auth-pam.cpp \
    authdialog.cpp \
    gsettings.cpp \
    auxiliary.cpp \
    configuration.cpp \
    screensaverwidget.cpp \
    screensaver.cpp \
    event_monitor.cpp \
    monitorwatcher.cpp

HEADERS += \
    mainwindow.h \
    unixsignallistener.h \
    auth-pam.h \
    auth.h \
    authdialog.h \
    gsettings.h \
    auxiliary.h \
    configuration.h \
    screensaverwidget.h \
    screensaver.h \
    event_monitor.h \
    monitorwatcher.h

FORMS += \
    mainwindow.ui \
    authdialog.ui

RESOURCES += \
    assets.qrc

TRANSLATIONS = ../i18n_ts/zh_CN.ts \
            ../i18n_ts/ru.ts \
            ../i18n_ts/fr.ts \
            ../i18n_ts/pt.ts \
            ../i18n_ts/es.ts

target.path = /usr/bin/

INSTALLS += target
