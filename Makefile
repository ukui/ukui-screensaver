# The default options are compiling with debugging information
mode = debug

CC  = gcc
CXX = g++

# Source file directory
I18N_SRC = i18n_ts/

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
all: gui i18n command

#
# Compilation
#

# Compile GUI
gui:
	qmake src/ $(QMAKE_OPTIONS) -o Saver_Makefile
	$(MAKE) -f Saver_Makefile
	qmake backend/ $(QMAKE_OPTIONS) -o Backend_Makefile
	$(MAKE) -f Backend_Makefile
	qmake command/ $(QMAKE_OPTIONS) -o Command_Makefile
	$(MAKE) -f Command_Makefile

# Generate Qt translation file
i18n:
	$(MAKE) -C $(I18N_SRC)

command:
	$(CC) src/ukui-screensaver-command.c -o ukui-screensaver-command


#
# Installation
#

install: install-target install-i18n install-data

install-target:
	# Install target
	$(MAKE) -f Saver_Makefile install INSTALL_ROOT=$(DESTDIR)/
	$(MAKE) -f Backend_Makefile install INSTALL_ROOT=$(DESTDIR)/
	$(MAKE) -f Command_Makefile install INSTALL_ROOT=$(DESTDIR)/

install-i18n:
	# Install i18n
	$(MAKE) -C $(I18N_SRC) install

install-data:
	# Install gsettings file
	install -D -m 644 data/org.ukui.screensaver.gschema.xml $(DESTDIR)$(GSETTINGS_DIR)/org.ukui.screensaver.gschema.xml
	install -D -m 644 data/ukui-screensaver.desktop $(DESTDIR)$(DESKTOP_DIR)/ukui-screensaver.desktop
	install -D -m 644 data/ukui-screensavers.menu $(DESTDIR)$(MENUS_DIR)/ukui-screensavers.menu
	install -D -m 644 data/ukui-screensaver.directory $(DESTDIR)$(DIRECTORY_DIR)/ukui-screensaver.directory


#
# Uninstallation
#

uninstall: uninstall-target uninstall-i18n uninstall-data

uninstall-target:
	# Uninstall target
	$(MAKE) -f Saver_Makefile uninstall
	$(MAKE) -f Backend_Makefile uninstall
	$(MAKE) -f Command_Makefile uninstall

uninstall-i18n:
	# Uninstall i18n
	$(MAKE) -C $(I18N_SRC) uninstall

uninstall-data:
	# Uninstall data
	rm -rf $(DESTDIR)$(GSETTINGS_DIR)/org.ukui.screensaver.gschema.xml
	rm -rf $(DESTDIR)$(DESKTOP_DIR)/ukui-screensaver.desktop
	rm -rf $(DESTDIR)$(MENUS_DIR)/ukui-screensavers.menu
	rm -rf $(DESTDIR)$(DIRECTORY_DIR)/ukui-screensaver.directory

#
# Clean intermediate file
#

clean:
	# Clean GUI intermediate files
	rm -f *.o moc_* ui_* qrc_*.cpp
	rm -f Saver_Makefile Backend_Makefile Command_Makefile .qmake.stash
	rm -f ukui-screensaver-dialog ukui-screensaver-command ukui-screensaver-backend
	# Clean i18n intermediate files
	$(MAKE) -C $(I18N_SRC) clean

