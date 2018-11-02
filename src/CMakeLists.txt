cmake_minimum_required(VERSION 3.0)

find_package(Qt5 COMPONENTS Core Widgets DBus X11Extras)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED gio-2.0>=2.40)
pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XEXT REQUIRED xext)
pkg_check_modules(XTST REQUIRED xtst)
pkg_check_modules(XCB REQUIRED xcb)

find_library(PAM_LIBRARIES pam)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(Debug ON)
if(Debug)
	set(CMAKE_BUILD_TYPE "Debug")
endif()
add_compile_options(-fPIC)

include_directories(${PROJECT_SOURCE_DIR}/src/VirtualKeyboard/src)
include_directories(${PROJECT_SOURCE_DIR}/src/bioAuthentication)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/bioAuthentication)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/VirtualKeyboard)

include_directories(
	${GIO_INCLUDE_DIRS}
	${X11_INCLUDE_DIRS}
	${XEXT_INCLUDE_DIRS}
	${XTST_INCLUDE_DIRS}
	${XCB_INCLUDE_DIRS}
	)

set(EXTRA_LIBS
	${EXTRA_LIBS}
	${PAM_LIBRARIES}
	${GIO_LIBRARIES}
	${X11_LIBRARIES}
	${XEXT_LIBRARIES}
	${XTST_LIBRARIES}
	${XCB_LIBRARIES}
	)

qt5_wrap_ui(dialog_SRC
	mainwindow.ui)

qt5_add_resources(dialog_SRC
	assets.qrc
	)

# 头文件中包含了Xlib.h，需要单独拿出来处理，不知道原因
qt5_wrap_cpp(dialog_SRC
	gsettings.h
	event_monitor.h
	)

set(dialog_SRC
	${dialog_SRC}
	main.cpp
	mainwindow.cpp
	unixsignallistener.cpp
	pam.cpp
	gsettings.cpp
	auxiliary.cpp
	configuration.cpp
	screensaverwidget.cpp
	screensaver.cpp
	event_monitor.cpp
	monitorwatcher.cpp
	)

add_executable(ukui-screensaver-dialog
	${dialog_SRC}
	$<TARGET_OBJECTS:VirtualKeyboard>
	$<TARGET_OBJECTS:bioAuthentication>
	)

target_link_libraries(ukui-screensaver-dialog
	Qt5::Core
	Qt5::Widgets
	Qt5::DBus
	Qt5::X11Extras
	${EXTRA_LIBS}
	)

set(backend_SRC
	backend_main.cpp
    interface.cpp
    sessionwatcher.cpp
    interfaceAdaptor.cpp
	)
add_executable(ukui-screensaver-backend ${backend_SRC})
target_link_libraries(ukui-screensaver-backend Qt5::Core Qt5::DBus)

set(command_SRC
	command_main.cpp)
add_executable(ukui-screensaver-command ${command_SRC})
target_link_libraries(ukui-screensaver-command Qt5::Core Qt5::DBus)


install(TARGETS
	ukui-screensaver-dialog
	ukui-screensaver-backend
	ukui-screensaver-command
	DESTINATION bin)
