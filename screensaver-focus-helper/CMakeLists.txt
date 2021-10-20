project(screensaver-capslock-helper)
  
set(CMAKE_AUTOMOC ON)

set(bin_SRCS
    ${bin_SRCS}
    main.cpp
    )

add_executable(screensaver-focus-helper ${bin_SRCS})
target_link_libraries(screensaver-focus-helper Qt5::Core  Qt5::Widgets)

install(TARGETS
        screensaver-focus-helper
	DESTINATION lib/ukui-screensaver)
