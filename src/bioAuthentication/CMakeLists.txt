qt5_add_resources(bioAuthentication_SRC
	bioverify.qrc)

aux_source_directory(. bioAuthentication_SRC)

include_directories(
	${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
	${Qt5DBus_INCLUDE_DIRS}
	)


add_library(bioAuthentication OBJECT ${bioAuthentication_SRC})

