datadir = ${prefix}/share/Xm/doc

ACLOCAL_AMFLAGS = -I m4

#data_DATA = BUGREPORT COPYRIGHT.MOTIF COPYING \
#            README RELEASE RELNOTES

EXTRA_DIST = BUGREPORT \
             COPYING \
             README RELEASE RELNOTES \
             INSTALL.configure Makefile.imake-pure

MAINTAINERCLEANFILES = Makefile.in \
             configure libtool \
             config.guess \
             config.sub \
             config.log \
             ltconfig ltmain.sh \
             aclocal.m4 \
             config.h.in \
             install-sh \
             missing mkinstalldirs \
             compile

SUBDIRS	= bindings bitmaps \
             config \
             localized \
             lib \
             include \
             tools \
             clients \
             doc \
	     $(MAYBE_DEMOS)
