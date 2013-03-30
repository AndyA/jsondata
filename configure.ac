AC_INIT([jsondata],[m4_esyscmd(sh tools/get-version.sh)])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_CONFIG_HEADER([config.h])

LT_INIT

AC_PROG_CC
AC_PROG_CC_C99
AC_SYS_LARGEFILE
BT_REQUIRE_PTHREAD

AC_MSG_CHECKING([whether to enable compiler warnings])
if test x"$GCC" = x"yes" ; then
   AC_MSG_RESULT([yes, -W -Wall -Werror])
   AM_CPPFLAGS="$_CPPFLAGS -W -Wall -Werror"
else
   AC_MSG_RESULT([no])
fi
AC_SUBST([AM_CPPFLAGS])

AC_CONFIG_FILES([
Makefile
t/Makefile
m4/Makefile
])

AC_OUTPUT
