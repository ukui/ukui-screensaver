INSTALL_DIR_DATA = /usr/share/ukui-screensaver

all: zh_CN.qm

zh_CN.qm:
	lrelease zh_CN.ts -qm zh_CN.qm

clean:
	rm -f zh_CN.qm

install:
	install -m 666 -D zh_CN.qm $(DESTDIR)$(INSTALL_DIR_DATA)/i18n_qm/zh_CN.qm

uninstall:
	rm -f $(DESTDIR)$(INSTALL_DIR_DATA)/i18n_qm/zh_CN.qm
