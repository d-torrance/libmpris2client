AC_PREREQ([2.64])
AC_INIT([libmpris2client], [0.1.0], [mati86dl@gmail.com], [], [https://github.com/matiasdelellis])
AC_CONFIG_SRCDIR([src/libmpris2client.c])
AC_CONFIG_HEADERS([config.h])
LT_INIT
AM_INIT_AUTOMAKE

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

# check for gtk-doc
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

# Checks for libraries.
PKG_CHECK_MODULES(GIO, gio-2.0, HAVE_GIO=yes, AC_MSG_ERROR([Could not find gio-2.0]))
PKG_CHECK_MODULES(GTK, gtk+-2.0, HAVE_GTK=yes, AC_MSG_ERROR([Could not find gtk+-2.0]))

# Checks for header files.
AC_CHECK_HEADERS([ctype.h stdlib.h string.h stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CONFIG_FILES([
Makefile
libmpris2client.pc
src/Makefile
status-icon/Makefile
docs/Makefile
docs/version.xml
])
AC_OUTPUT

echo "
  CONFIGURATION INFO FOR ${PACKAGE} ${VERSION}
    Prefix:                        ${prefix}
    Libbir:                        ${libdir}
    CFLAGS:                        ${CFLAGS}
    CXXFLAGS:                      ${CXXFLAGS}
"