MAINTAINERCLEANFILES = Makefile.in

bindir = ${prefix}/share/Xm/Column
datadir = ${prefix}/share/Xm/Column

bin_PROGRAMS = column
column_SOURCES = column.c
EXTRA_DIST = Column.ad
data_DATA = $(EXTRA_DIST) $(column_SOURCES)
TOPDIR = ../../..
LDADD = $(TOPDIR)/lib/Xm/libXm.la -lXt -lX11
INCLUDES = -I$(TOPDIR)/lib -I$(srcdir)/$(TOPDIR)/lib ${X_CFLAGS} 
