cmake_minimum_required(VERSION 2.6)
project(ukui-screensaver)

find_package(Qt5 COMPONENTS Core Widgets DBus X11Extras Xml)
find_package(PkgConfig REQUIRED)

set(TS_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/zh_CN.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/es.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/fr.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/pt.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/ru.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/tr.ts
	)
add_custom_command(
	OUTPUT ${TS_FILES}
	COMMAND lupdate src/ screensaver/ -ts ${TS_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
add_custom_target(
	i18n_ts
	DEPENDS ${TS_FILES}
	)
add_compile_options(-fPIC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_PREFIX /usr)

set(Debug ON)
if(Debug)
    set(CMAKE_BUILD_TYPE "Debug")                                                                                                     
endif()

#add_subdirectory(BioAuth)
add_subdirectory(BiometricAuth)
add_subdirectory(VirtualKeyboard)
add_subdirectory(src)
add_subdirectory(i18n_ts)
add_subdirectory(data)
add_subdirectory(screensaver)
add_subdirectory(Common)

add_dependencies(ukui-screensaver-dialog BiometricAuth VirtualKeyboard Common)
add_dependencies(ukui-screensaver-default Common)
