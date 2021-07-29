qt5_wrap_cpp(BiometricAuth_SRC
	biometricdeviceinfo.h
	biometricproxy.h
	biometricauthwidget.h
	biometricdeviceswidget.h
	giodbus.h
	)

set(BiometricAuth_SRC
	${BiometricAuth_SRC}
	biometricdeviceinfo.cpp
	biometricproxy.cpp
	biometricauthwidget.cpp
	biometricdeviceswidget.cpp
	giodbus.cpp
	)

include_directories(
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	${Qt5DBus_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${GLIB2_INCLUDE_DIRS}
	)


add_library(BiometricAuth STATIC ${BiometricAuth_SRC})
target_link_libraries(BiometricAuth Qt5::Core Qt5::DBus Qt5::Widgets ${OpenCV_LIBS} ${GIOUNIX2_LIBRARIES})
