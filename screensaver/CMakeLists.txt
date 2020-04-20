pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XTST REQUIRED xtst)
pkg_check_modules(QGS REQUIRED gsettings-qt)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/Common)

include_directories(
	${X11_INCLUDE_DIRS}
        ${XTST_INCLUDE_DIRS}
	${QGS_INCLUDE_DIRS}
	)

set(EXTRA_LIBS
	${EXTRA_LIBS}
	${X11_LIBRARIES}
        ${XTST_LIBRARIES}
	${QGS_LIBRARIES}
	Common
	)
qt5_add_resources(screensaver_SRC
	default.qrc
	)

qt5_wrap_cpp(screensaver_SRC
    chinesedate.h
    screensaver.h
    mbackground.h
    sleeptime.h
    )
set(screensaver_SRC
    ${screensaver_SRC}
    main.cpp
    chinesedate.cpp
    mbackground.cpp
    screensaver.cpp
    sleeptime.cpp
    )
add_executable(ukui-screensaver-default ${screensaver_SRC})
target_link_libraries(ukui-screensaver-default Qt5::Core Qt5::Widgets Qt5::X11Extras Qt5::Xml ${EXTRA_LIBS})

install(TARGETS
	ukui-screensaver-default
	DESTINATION lib/ukui-screensaver)

install(FILES
        screensaver.ini 
        DESTINATION share/ukui-screensaver)
