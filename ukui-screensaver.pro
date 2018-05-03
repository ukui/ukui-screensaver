#-------------------------------------------------
#
# Project created by QtCreator 2018-04-09T11:40:40
#
#-------------------------------------------------

TRANSLATIONS = i18n_ts/zh_CN.ts
QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ukui-screensaver
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

LIBS += -lpam -lpam_misc

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
	main.cpp \
	mainwindow.cpp \
    unixsignallistener.cpp \
    pam.cpp \
    gsettings.cpp \
    auxiliary.cpp \
    configuration.cpp

HEADERS += \
	mainwindow.h \
    unixsignallistener.h \
    pam.h \
    gsettings.h \
    auxiliary.h \
    configuration.h

FORMS += \
	mainwindow.ui
