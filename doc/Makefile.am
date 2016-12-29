NULL =

SPEC_XML_FILES =			\
	kylin-screensaver.xml		\
	dbus-interface.xml		\
	$(NULL)

if DOCBOOK_DOCS_ENABLED

htmldocdir = $(DOCDIR)
htmldoc_DATA = kylin-screensaver.html

kylin-screensaver.html : dbus-interface.xml
	$(XMLTO) html-nochunks -m config.xsl kylin-screensaver.xml

endif # DOCBOOK_DOCS_ENABLED

EXTRA_DIST =				\
	kylin-screensaver.xml.in	\
	config.xsl			\
	docbook.css			\
	$(SPEC_XML_FILES)
	$(NULL)

clean-local:
	rm -f *~
	rm -f kylin-screensaver.html
