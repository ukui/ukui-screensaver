NULL =

BACKGROUNDS = \
	blue-marble-west-ukui.jpg	\
	cloud-ukui.jpg 		\
	comet-ukui.jpg 		\
	earth-horizon-ukui.jpg	\
	galaxy-ngc3370-ukui.jpg	\
	helix-nebula-ukui.jpg	\
	jupiter-ukui.jpg		\
	sombrero-ukui.jpg		\
	whirlpool-ukui.jpg		\
	$(NULL)

imagesdir   = $(SLIDESHOW_COSMOS_DIR)
images_DATA =			\
	$(BACKGROUNDS)		\
	background-1-ukui.xml	\
	$(NULL)

cosmos.xml.in: cosmos.xml.in.in Makefile
	$(edit) $< >$@

background-1-ukui.xml: background-1-ukui.xml.in Makefile
	$(edit) $< >$@

metadata_in_files = cosmos.xml.in
metadatadir = $(datadir)/ukui-background-properties
metadata_DATA = $(metadata_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

edit = sed \
	-e 's|@IMAGESDIR[@]|$(imagesdir)|g'

EXTRA_DIST =			\
	CREDITS 		\
	$(BACKGROUNDS)		\
	background-1-ukui.xml.in	\
	cosmos.xml.in.in	\
	$(NULL)

CLEANFILES = \
	cosmos.xml.in			\
	$(NULL)

DISTCLEANFILES = \
	cosmos.xml			\
	background-1-ukui.xml		\
	$(NULL)

MAINTAINERCLEANFILES =                  \
	*~				\
	Makefile.in			\
	$(NULL)
