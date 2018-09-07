INSTALL_DIR_MAN = /usr/share/man/man1

all:
	gzip < ukui-screensaver.1 > ukui-screensaver.1.gz
	gzip < ukui-screensaver-command.1 > ukui-screensaver-command.1.gz

install:
	install -m 644 -D ukui-screensaver.1.gz $(DESTDIR)$(INSTALL_DIR_MAN)/ukui-screensaver.1.gz
	install -m 644 -D ukui-screensaver-command.1.gz $(DESTDIR)$(INSTALL_DIR_MAN)/ukui-screensaver-command.1.gz

uninstall:
	rm -f $(DESTDIR)$(INSTALL_DIR_MAN)/ukui-screensaver.1.gz
	rm -f $(DESTDIR)$(INSTALL_DIR_MAN)/ukui-screensaver-command.1.gz

clean:
	rm -f ukui-screensaver.1.gz ukui-screensaver-command.1.gz
