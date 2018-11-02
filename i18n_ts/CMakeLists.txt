find_package(Qt5LinguistTools)

file(GLOB ts_files *.ts)
qt5_create_translation(qm_files ${ts_files})
add_custom_target(i18n
	DEPENDS ${qm_files}
	SOURCES ${ts_files}
	)
# 让主目标依赖翻译文件，这样才会执行i18n
add_dependencies(ukui-screensaver-dialog i18n)

install(FILES ${qm_files} DESTINATION /usr/share/ukui-screensaver/i18n_qm/)
