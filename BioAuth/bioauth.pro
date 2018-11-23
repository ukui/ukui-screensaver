QT  += core gui dbus

TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += staticlib debug c++11

DESTDIR = $$PWD/

include(bioauth.pri)

INCLUDEPATH += $$PWD/include/ \
               $$PWD/../common/

HEADERS += \
    include/biodevices.h \
    include/bioauth.h \
    include/biotypes.h \
    include/bioauthwidget.h \
    include/biodeviceswidget.h \
    include/biometric.h

SOURCES += \
    src/biodevices.cpp \
    src/biotypes.cpp \
    src/bioauthwidget.cpp \
    src/biodeviceswidget.cpp \
    src/bioauth.cpp

FORMS += \
    src/bioauthwidget.ui \
    src/biodeviceswidget.ui

TRANSLATIONS += i18n_ts/zh_CN.ts \
                i18n_ts/fr.ts \
                i18n_ts/pt.ts \
                i18n_ts/ru.ts \
                i18n_ts/es.ts

system(lrelease i18n_ts/*.ts)

qm_file.files = i18n_ts/*.qm
qm_file.path = /${UKUI_BIOMETRIC}/i18n_qm/

INSTALLS += qm_file
