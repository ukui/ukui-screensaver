# The default options are compiling with debugging information
mode = debug

CC  = gcc
CXX = g++

# Source file directory
I18N_SRC = i18n_ts/
MAN_SRC = man/

# bin file installation directory
BIN_DIR = /usr/bin

# gsettings xml file
GSETTINGS_DIR = /usr/share/glib-2.0/schemas

# desktop file
DESKTOP_DIR = /etc/xdg/autostart

# directory file
DIRECTORY_DIR = /usr/share/desktop-directories

# menus file
MENUS_DIR = /etc/xdg/menus

# Set build options
ifeq ($(mode), debug)
	# Compile with debugging information
	QMAKE_OPTIONS = CONFIG+=debug
else
	# Compile without debugging information
	QMAKE_OPTIONS =
endif

# Target
all: gui i18n command man

#
# Compilation
#

# Compile GUI
gui:
	qmake $(QMAKE_OPTIONS) -o QtMakefile
	make -f QtMakefile

# Generate Qt translation file
i18n:
	$(MAKE) -C $(I18N_SRC)

command:
	$(CC) src/ukui-screensaver-command.c -o ukui-screensaver-command

man:
	$(MAKE) -C $(MAN_SRC)

#
# Installation
#

install: install-gui install-i18n install-data install-man

install-gui:
	# Install GUI
	install -D ukui-screensaver $(DESTDIR)$(BIN_DIR)/ukui-screensaver
	install -D ukui-screensaver-command $(DESTDIR)$(BIN_DIR)/ukui-screensaver-command

install-i18n:
	# Install i18n
	$(MAKE) -C $(I18N_SRC) install

install-data:
	# Install gsettings file
	install -D -m 644 data/org.ukui.screensaver.gschema.xml $(DESTDIR)$(GSETTINGS_DIR)/org.ukui.screensaver.gschema.xml
	install -D -m 644 data/ukui-screensaver.desktop $(DESTDIR)$(DESKTOP_DIR)/ukui-screensaver.desktop
	install -D -m 644 data/ukui-screensavers.menu $(DESTDIR)$(MENUS_DIR)/ukui-screensavers.menu
	install -D -m 644 data/ukui-screensaver.directory $(DESTDIR)$(DIRECTORY_DIR)/ukui-screensaver.directory

install-man:
	$(MAKE) -C $(MAN_SRC) install

#
# Uninstallation
#

uninstall: uninstall-gui uninstall-i18n uninstall-data

uninstall-gui:
	# Uninstall GUI and scripts
	rm -rf $(DESTDIR)$(BIN_DIR)/ukui-screensaver
	rm -rf $(DESTDIR)$(BIN_DIR)/ukui-screensaver-command

uninstall-i18n:
	# Uninstall i18n
	$(MAKE) -C $(I18N_SRC) uninstall

uninstall-data:
	# Uninstall data
	rm -rf $(DESTDIR)$(GSETTINGS_DIR)/org.ukui.screensaver.gschema.xml
	rm -rf $(DESTDIR)$(DESKTOP_DIR)/ukui-screensaver.desktop
	rm -rf $(DESTDIR)$(MENUS_DIR)/ukui-screensavers.menu
	rm -rf $(DESTDIR)$(DIRECTORY_DIR)/ukui-screensaver.directory

uninstall-man:
	# Uinstall man
	$(MAKE) -C $(MAN_SRC) uninstall
#
# Clean intermediate file
#

clean:
	# Clean GUI intermediate files
	rm -f *.o moc_* ui_* qrc_*.cpp
	rm -f QtMakefile .qmake.stash ukui-screensaver ukui-screensaver-command
	# Clean i18n intermediate files
	$(MAKE) -C $(I18N_SRC) clean
	$(MAKE) -C $(MAN_SRC) clean

.PHONY: clean man
