cmake_minimum_required(VERSION 2.6)
project(ukui-screensaver)

add_subdirectory(src)
add_subdirectory(i18n_ts)
add_subdirectory(data)

set(TS_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/zh_CN.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/es.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/fr.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/pt.ts
	${CMAKE_CURRENT_SOURCE_DIR}/i18n_ts/ru.ts
	)
add_custom_command(
	OUTPUT ${TS_FILES}
	COMMAND lupdate src/ -ts ${TS_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
add_custom_target(
	i18n_ts
	DEPENDS ${TS_FILES}
	)
