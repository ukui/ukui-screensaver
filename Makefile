# The default options are compiling with debugging information
mode = debug

# Source file directory
I18N_SRC = i18n_ts/

# QDBus XML file and the installation directory
INSTALL_DIR = /usr/local/ukui-screensaver

# Set build options
ifeq ($(mode), debug)
	# Compile with debugging information
	QMAKE_OPTIONS = CONFIG+=debug
else
	# Compile without debugging information
	QMAKE_OPTIONS =
endif

# Target
all: gui i18n

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

#
# Installation
#

install: install-gui install-i18n

install-gui:
	# Install GUI
	install -D ukui-screensaver $(DESTDIR)$(INSTALL_DIR)/ukui-screensaver

install-i18n:
	# Install i18n
	$(MAKE) -C $(I18N_SRC) install

#
# Uninstallation
#

uninstall: uninstall-gui uninstall-i18n

uninstall-gui:
	# Uninstall GUI and scripts
	rm -rf $(DESTDIR)$(INSTALL_DIR)

uninstall-i18n:
	# Uninstall i18n
	$(MAKE) -C $(I18N_SRC) uninstall

#
# Clean intermediate file
#

clean:
	# Clean GUI intermediate files
	rm -f *.o moc_* ui_*
	rm -f QtMakefile .qmake.stash ukui-screensaver
	# Clean i18n intermediate files
	$(MAKE) -C $(I18N_SRC) clean
