MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = README wmllex.c wmllex.l motif.wml Uil.y Uil.c Uil.h UilLexPars.h UilLexPars.c

AM_YFLAGS = -d
#AM_LFLAGS = -l

WMLTARGETS = UilConst.h	UilKeyTab.h	 \
	UilSymArTa.h	UilSymArTy.h	UilSymCSet.h	UilSymCtl.h \
	UilSymEnum.h	UilSymGen.h	UilSymNam.h	UilSymRArg.h \
	UilSymReas.h	UilTokName.h	UilUrmClas.h	UilSymChCl.h \
	UilSymChTa.h

TABLE = motif.wml
WMDTABLE = motif.wmd
WMLTOOLS = wml wmluiltok
REPORT = wml.report

wmldir = $(includedir)/uil

wml_HEADERS = UilDBDef.h

noinst_HEADERS = wml.h wmlparse.h $(WMLTARGETS)

noinst_PROGRAMS = wml wmluiltok wmldbcreate
wmluiltok_SOURCES = wmluiltok.l
wmluiltok_LDADD = @LEXLIB@

wml_SOURCES = wml.c
wml_LDADD = -L. -lwml
wml_DEPENDENCIES = libwml.a

wmldbcreate_LDADD = ../../lib/Xm/libXm.la $(XTOOLLIB)

AM_CPPFLAGS = -I../../lib -I../../include ${X_CFLAGS} 

noinst_LIBRARIES = libwml.a
libwml_a_SOURCES = wmlparse.y wml.c wmloutkey.c wmlouth.c wmloutmm.c wmloutp1.c wmlresolve.c wmlsynbld.c wmlutils.c

wmlsynbld.c: wmlparse.h

wmlparse.c: wmllex.c

wmllex.c: wmllex.l

wmlsynbld.c: wmlparse.h

wmldbcreate.c: $(WMLTARGETS) UilLexPars.h

UilLexPars.h: UilLexPars.c

UilLexPars.c: Uil.c
	$(LN_S) Uil.h UilLexPars.h
	$(LN_S) Uil.c UilLexPars.c

$(WMLTARGETS): wml-uil.mm

wml-uil.mm: wmluiltok wml UilLexPars.c $(TABLE)
	$(RM) $(WMLTARGETS) wml.report wml-uil.mm
	./wmluiltok <$(abs_srcdir)/Uil.y >tokens.dat
	$(RM) wml-uil.mm
	./wml $(abs_srcdir)/$(TABLE)
	$(RM) tokens.dat

all: motif.wmd

motif.wmd: wmldbcreate
	./wmldbcreate -o motif.wmd

clean-local:
	$(RM) $(WMLTARGETS) $(WMDTABLE) $(REPORT) lex.yy.c libwml.a wml \
	*.mm *.sdml *.txt wmlparse.c wmlparse.h wmluiltok wmllex.c \
	tokens.dat DONE wmluiltok.c Uil.c Uil.h wml.report UilLexPars.[ch] ylwrap
 
