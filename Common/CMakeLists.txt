qt5_wrap_cpp(Common_SRC
	autoresize.h
        checkbutton.h
	)

set(Common_SRC
	${Common_SRC}
	autoresize.cpp
        checkbutton.cpp
	)

include_directories(
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5DBus_INCLUDE_DIRS}
	)


add_library(Common STATIC ${Common_SRC})
target_link_libraries(Common Qt5::Core Qt5::DBus Qt5::Widgets)
