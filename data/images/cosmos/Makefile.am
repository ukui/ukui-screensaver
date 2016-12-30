NULL =

BACKGROUNDS = \
	blue-marble-west-kylin.jpg	\
	cloud-kylin.jpg 		\
	comet-kylin.jpg 		\
	earth-horizon-kylin.jpg	\
	galaxy-ngc3370-kylin.jpg	\
	helix-nebula-kylin.jpg	\
	jupiter-kylin.jpg		\
	sombrero-kylin.jpg		\
	whirlpool-kylin.jpg		\
	$(NULL)

imagesdir   = $(SLIDESHOW_COSMOS_DIR)
images_DATA =			\
	$(BACKGROUNDS)		\
	background-1-kylin.xml	\
	$(NULL)

cosmos.xml.in: cosmos.xml.in.in Makefile
	$(edit) $< >$@

background-1-kylin.xml: background-1-kylin.xml.in Makefile
	$(edit) $< >$@

metadata_in_files = cosmos.xml.in
metadatadir = $(datadir)/kylin-background-properties
metadata_DATA = $(metadata_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

edit = sed \
	-e 's|@IMAGESDIR[@]|$(imagesdir)|g'

EXTRA_DIST =			\
	CREDITS 		\
	$(BACKGROUNDS)		\
	background-1-kylin.xml.in	\
	cosmos.xml.in.in	\
	$(NULL)

CLEANFILES = \
	cosmos.xml.in			\
	$(NULL)

DISTCLEANFILES = \
	cosmos.xml			\
	background-1-kylin.xml		\
	$(NULL)

MAINTAINERCLEANFILES =                  \
	*~				\
	Makefile.in			\
	$(NULL)
