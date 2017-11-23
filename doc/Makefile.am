NULL =

SPEC_XML_FILES =			\
	ukui-screensaver.xml		\
	dbus-interface.xml		\
	$(NULL)

if DOCBOOK_DOCS_ENABLED

htmldocdir = $(DOCDIR)
htmldoc_DATA = ukui-screensaver.html

ukui-screensaver.html : dbus-interface.xml
	$(XMLTO) html-nochunks -m config.xsl ukui-screensaver.xml

endif # DOCBOOK_DOCS_ENABLED

EXTRA_DIST =				\
	ukui-screensaver.xml.in	\
	config.xsl			\
	docbook.css			\
	$(SPEC_XML_FILES)
	$(NULL)

clean-local:
	rm -f *~
	rm -f ukui-screensaver.html
