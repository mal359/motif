AC_DEFUN([AC_FIND_XBITMAPS],
[

AC_CHECK_HEADERS(X11/bitmaps/gray)

CFLAGS="$CFLAGS `pkg-config xbitmaps --cflags`"
AC_SUBST(CFLAGS)
LIBS="$LIBS `pkg-config xbitmaps --libs`"
AC_SUBST(LIBS)
])
