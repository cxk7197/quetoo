
bin_PROGRAMS = \
	quetoo-master

quetoo_master_SOURCES = \
	main.c

quetoo_master_CFLAGS = \
	-I$(top_srcdir)/src \
	@BASE_CFLAGS@ \
	@GLIB_CFLAGS@

quetoo_master_LDADD = \
	$(top_builddir)/src/libfilesystem.la \
	$(top_builddir)/src/libparse.la
