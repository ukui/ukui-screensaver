pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XTST REQUIRED xtst)
pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(QGS REQUIRED gsettings-qt)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(MMIX REQUIRED libmatemixer)

find_library(PAM_LIBRARIES pam)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/VirtualKeyboard/src)
include_directories(${PROJECT_SOURCE_DIR}/BiometricAuth)
include_directories(${PROJECT_SOURCE_DIR}/Common)
include_directories(${PROJECT_SOURCE_DIR}/KylinNM)
include_directories(${PROJECT_SOURCE_DIR}/KylinNM/src)
include_directories(${PROJECT_SOURCE_DIR}/KylinNM/hot-spot)
include_directories(${PROJECT_SOURCE_DIR}/KylinNM/wireless-security)

include_directories(
    ${X11_INCLUDE_DIRS}
    ${XTST_INCLUDE_DIRS}
    ${XCB_INCLUDE_DIRS}
    ${QGS_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${MMIX_INCLUDE_DIRS}
    )

set(EXTRA_LIBS
    ${EXTRA_LIBS}
    ${PAM_LIBRARIES}
    ${X11_LIBRARIES}
    ${XTST_LIBRARIES}
    ${XCB_LIBRARIES}
    ${QGS_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${MMIX_LIBRARIES}
    -lrt
    )

qt5_wrap_ui(dialog_SRC
    lockwidget.ui
    )

qt5_add_resources(dialog_SRC
    assets.qrc
    ../KylinNM/nmqrc.qrc  #暂时将麒麟网络的资源文件放到这里，否则显示不出来，暂时不知道原因
    )

# 头文件中包含了Xlib.h，需要单独拿出来处理，不知道原因
qt5_wrap_cpp(dialog_SRC
    pam-tally.h
    fullbackgroundwidget.h
    lockwidget.h
    authdialog.h
    screensaverwidget.h
    auth.h
    auth-pam.h
    screensaver.h
    xeventmonitor.h
    monitorwatcher.h
    configuration.h
    users.h
    displaymanager.h
    iconedit.h
    powermanager.h
    logind.h
    weathermanager.h
    tabletlockwidget.h
    gestureauthdialog.h
    digitalauthdialog.h
    switchbuttongroup.h
    wechatauthdialog.h
    verificationwidget.h
    verticalVerificationwidget.h
    common.h
    eduplatforminterface.h
    accountsinterface.h
    networkwatcher.h
    digitalkeyboard.h
    PhysicalDeviceSet/brightnessdeviceset.h
    PhysicalDeviceSet/flightmodeset.h
    PhysicalDeviceSet/sounddeviceset.h
    PhysicalDeviceSet/touchscreenset.h
    )

set(dialog_SRC
    ${dialog_SRC}
    ukui-screensaver-dialog.cpp
    fullbackgroundwidget.cpp
    lockwidget.cpp
    authdialog.cpp
    screensaverwidget.cpp
    auth-pam.cpp
    xeventmonitor.cpp
    monitorwatcher.cpp
    grab-x11.cpp
    configuration.cpp
    screensaver.cpp
    powermanager.cpp
    users.cpp
    displaymanager.cpp
    iconedit.cpp
    logind.cpp
    weathermanager.cpp
    tabletlockwidget.cpp
    gestureauthdialog.cpp
    digitalauthdialog.cpp
    switchbuttongroup.cpp
    wechatauthdialog.cpp
    verificationwidget.cpp
    verticalVerificationwidget.cpp
    eduplatforminterface.cpp
    accountsinterface.cpp
    networkwatcher.cpp
    digitalkeyboard.cpp
    PhysicalDeviceSet/brightnessdeviceset.cpp
    PhysicalDeviceSet/flightmodeset.cpp
    PhysicalDeviceSet/sounddeviceset.cpp
    PhysicalDeviceSet/touchscreenset.cpp
    )
add_executable(ukui-screensaver-dialog ${dialog_SRC})

target_link_libraries(ukui-screensaver-dialog
	Qt5::Core
	Qt5::Widgets
	Qt5::DBus
	Qt5::Svg
	Qt5::X11Extras
        Qt5::Network
	${EXTRA_LIBS}
        BiometricAuth 
	VirtualKeyboard 
	Common
        Kylin-nm
	)
link_libraries(libmatemixer.so glib-2.0.so)

qt5_add_dbus_adaptor(backend_SRC
    org.ukui.ScreenSaver.xml
    interface.h
    Interface
    )

qt5_wrap_cpp(backend_SRC
    interface.h
    sessionwatcher.h
    logind.h
    )
set(backend_SRC
    ${backend_SRC}
    ukui-screensaver-backend.cpp
    interface.cpp
    sessionwatcher.cpp
    logind.cpp
    )
add_executable(ukui-screensaver-backend ${backend_SRC})
target_link_libraries(ukui-screensaver-backend Qt5::Core Qt5::DBus ${QGS_LIBRARIES})

set(command_SRC
    ukui-screensaver-command.cpp
    )
add_executable(ukui-screensaver-command ${command_SRC})
target_link_libraries(ukui-screensaver-command Qt5::Core Qt5::DBus)

set(checkpass_SRC
    ukui-screensaver-checkpass.cpp
    )
add_executable(ukui-screensaver-checkpass ${checkpass_SRC})
target_link_libraries(ukui-screensaver-checkpass ${PAM_LIBRARIES})

install(TARGETS
    ukui-screensaver-dialog
    ukui-screensaver-backend
    ukui-screensaver-command
    ukui-screensaver-checkpass
    DESTINATION bin)

#set(test-act_SRC
#	users.cpp
#	test-accounts.cpp
#	)
#add_executable(test-accounts ${test-act_SRC})
#target_link_libraries(test-accounts Qt5::Core Qt5::DBus)
