pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XTST REQUIRED xtst)
pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(QGS REQUIRED gsettings-qt)

find_library(PAM_LIBRARIES pam)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/VirtualKeyboard/src)
include_directories(${PROJECT_SOURCE_DIR}/BiometricAuth)
include_directories(${PROJECT_SOURCE_DIR}/Common)

include_directories(
	${X11_INCLUDE_DIRS}
	${XTST_INCLUDE_DIRS}
	${XCB_INCLUDE_DIRS}
	${QGS_INCLUDE_DIRS}
	)

set(EXTRA_LIBS
	${EXTRA_LIBS}
	${PAM_LIBRARIES}
	${X11_LIBRARIES}
	${XTST_LIBRARIES}
	${XCB_LIBRARIES}
	${QGS_LIBRARIES}
        -lrt
	)

qt5_wrap_ui(dialog_SRC
        lockwidget.ui
        )

qt5_add_resources(dialog_SRC
	assets.qrc
	)

# 头文件中包含了Xlib.h，需要单独拿出来处理，不知道原因
qt5_wrap_cpp(dialog_SRC
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
    pam-tally.h
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
    pam-tally.cpp
	)
add_executable(ukui-screensaver-dialog ${dialog_SRC})

target_link_libraries(ukui-screensaver-dialog
	Qt5::Core
	Qt5::Widgets
	Qt5::DBus
	Qt5::X11Extras
	${EXTRA_LIBS}
    BiometricAuth 
	VirtualKeyboard 
	Common
	)

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


install(TARGETS
	ukui-screensaver-dialog
	ukui-screensaver-backend
	ukui-screensaver-command
	DESTINATION bin)

#set(test-act_SRC
#	users.cpp
#	test-accounts.cpp
#	)
#add_executable(test-accounts ${test-act_SRC})
#target_link_libraries(test-accounts Qt5::Core Qt5::DBus)
