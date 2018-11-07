qt5_wrap_ui(BioAuth_SRC
	src/bioauthwidget.ui
	src/biodeviceswidget.ui
	)

qt5_wrap_cpp(BioAuth_SRC
	include/bioauth.h
	include/biodevices.h
	include/biotypes.h
	include/biometric.h
	include/biodeviceswidget.h
	)
qt5_add_resources(BioAuth_SRC
	bio-assets.qrc
	)

set(BioAuth_SRC
	${BioAuth_SRC}
	src/bioauth.cpp
	src/biodevices.cpp
	src/biotypes.cpp
	src/biodeviceswidget.cpp
	)

include_directories(
	${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
	${Qt5DBus_INCLUDE_DIRS}
	include
	)


add_library(BioAuth OBJECT ${BioAuth_SRC})

set(test_SRC
	${BioAuth_SRC}
	tests/test_biodeviceswidget.cpp
	)

add_executable(test-devicewidget ${test_SRC})
target_link_libraries(test-devicewidget
	Qt5::Core
	Qt5::Widgets
	Qt5::DBus
	)
