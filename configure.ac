AC_INIT([jsondata],[m4_esyscmd(sh tools/get-version.sh)])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_CONFIG_HEADER([config.h])

BT_PROG_XCODE
AC_PROG_CC([clang gcc c99 cc])


AC_ARG_ENABLE(dtrace,
  [AS_HELP_STRING([--enable-dtrace],[Enable dtrace probes])])
if test "x$enable_dtrace" = "xyes"; then
  AC_PATH_PROG([DTRACE], [dtrace], "no", [/usr/sbin:$PATH])
  if test "x$DTRACE" != "xno"; then
    AC_DEFINE([ENABLE_DTRACE],1,[Set to nonzero if you want to include DTRACE])
    build_dtrace=yes
    # DTrace on MacOSX does not use -G option
    $DTRACE -G -o conftest.$$ -s jd_dtrace.d 2>/dev/null
    if test $? -eq 0
    then
        dtrace_instrument_obj=yes
        rm conftest.$$
    fi

    if test "`which tr`" = "/usr/ucb/tr"; then
        AC_MSG_ERROR([Please remove /usr/ucb from your path. See man standards for more info])
    fi
  else
    AC_MSG_ERROR([Need dtrace binary and OS support.])
  fi
fi

AM_CONDITIONAL([BUILD_DTRACE],[test "$build_dtrace" = "yes"])
AM_CONDITIONAL([DTRACE_INSTRUMENT_OBJ],[test "$dtrace_instrument_obj" = "yes"])

AC_SUBST(DTRACE)
AC_SUBST(DTRACEFLAGS)

LT_INIT

AC_PROG_CC_C99
AC_SYS_LARGEFILE
BT_REQUIRE_PTHREAD
BT_PROG_CC_WARN

AC_CONFIG_FILES([
Makefile
bm/Makefile
t/Makefile
m4/Makefile
])

AC_OUTPUT
