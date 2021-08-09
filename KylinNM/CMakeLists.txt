find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTORCC ON)
#pkg_check_modules(X11 REQUIRED x11)

qt5_wrap_ui(Kylin_NM_SRC
    src/confform.ui
    src/kylinnm.ui
    src/oneconnform.ui
    src/onelancform.ui
    wireless-security/dlgconnhidwifi.ui
    wireless-security/dlgconnhidwifisecfast.ui
    wireless-security/dlgconnhidwifisectunneltls.ui
    wireless-security/dlgconnhidwifisecpeap.ui
    wireless-security/dlgconnhidwifisectls.ui
    wireless-security/dlgconnhidwifisecleap.ui
    wireless-security/dlgconnhidwifisecpwd.ui
    wireless-security/dlgconnhidwifiwep.ui
    wireless-security/dlgconnhidwifileap.ui
    wireless-security/dlgconnhidwifiwpa.ui
    hot-spot/dlghotspotcreate.ui
    )

#qt5_wrap_cpp(Kylin_NM_SRC
#    src/backthread.h
#    src/confform.h
#    src/ksimplenm.h
#    src/kylin-dbus-interface.h
#    src/kylin-network-interface.h
#    src/loadingdiv.h
#    src/kylinnm.h
#    src/oneconnform.h
#    src/onelancform.h
#    src/switchbutton.h
#    src/utils.h
#    wireless-security/dlgconnhidwifi.h
#    wireless-security/dlgconnhidwifisecfast.h
#    wireless-security/dlgconnhidwifisectunneltls.h
#    wireless-security/dlgconnhidwifisecpeap.h
#    wireless-security/dlgconnhidwifisectls.h
#    wireless-security/dlgconnhidwifisecleap.h
#    wireless-security/dlgconnhidwifisecpwd.h
#    wireless-security/dlgconnhidwifiwep.h
#    wireless-security/dlgconnhidwifileap.h
#    wireless-security/dlgconnhidwifiwpa.h
#    wireless-security/kylinheadfile.h
#    hot-spot/dlghotspotcreate.h
#    )

qt5_add_resources(Kylin_NM_SRC
    nmqrc.qrc
    res.qrc)

set(Kylin_NM_SRC
    ${Kylin_NM_SRC}
    src/backthread.cpp
    src/confform.cpp
    src/ksimplenm.cpp
    src/kylin-dbus-interface.cpp
    src/kylin-network-interface.c
    src/loadingdiv.cpp
    src/kylinnm.cpp
    src/oneconnform.cpp
    src/onelancform.cpp
    src/switchbutton.cpp
    src/utils.cpp
    src/swipegesturerecognizer.cpp
    wireless-security/dlgconnhidwifi.cpp
    wireless-security/dlgconnhidwifisecfast.cpp
    wireless-security/dlgconnhidwifisectunneltls.cpp
    wireless-security/dlgconnhidwifisecpeap.cpp
    wireless-security/dlgconnhidwifisectls.cpp
    wireless-security/dlgconnhidwifisecleap.cpp
    wireless-security/dlgconnhidwifisecpwd.cpp
    wireless-security/dlgconnhidwifiwep.cpp
    wireless-security/dlgconnhidwifileap.cpp
    wireless-security/dlgconnhidwifiwpa.cpp
    wireless-security/kylinheadfile.cpp
    hot-spot/dlghotspotcreate.cpp
    nmqrc.qrc
    )

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    )

find_package(KF5WindowSystem)

add_library(Kylin-nm STATIC ${Kylin_NM_SRC})
target_link_libraries(Kylin-nm Qt5::Core Qt5::Widgets KF5::WindowSystem Qt5::DBus Qt5::X11Extras)
