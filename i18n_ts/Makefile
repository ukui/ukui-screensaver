INSTALL_DIR_DATA = /usr/share/ukui-screensaver

all:
	lrelease *.ts

clean:
	rm -f *.qm

install:
	mkdir -p $(DESTDIR)$(INSTALL_DIR_DATA)/i18n_qm/
	install -m 666 -D *.qm $(DESTDIR)$(INSTALL_DIR_DATA)/i18n_qm/

uninstall:
	rm -f $(DESTDIR)$(INSTALL_DIR_DATA)/i18n_qm/*.qm
