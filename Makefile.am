DIST_SUBDIRS = bm t m4

SUBDIRS = . bm t

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libjsondata.la

noinst_PROGRAMS = jdtest

libjsondata_la_SOURCES = jsondata.h jd_private.h \
	jsondata.c jd_string.c jd_array.c jd_hash.c \
	jd_path.c jd_json.c jd_closure.c jd_object.c \
	jd_printf.c jd_exception.c jd_map.c jd_magic.c \
	jd_utf8.c

libjsondata_la_LDFLAGS = -avoid-version

jdtest_SOURCES = jdtest.c

jdtest_LDADD = libjsondata.la

test: all
	cd t && $(MAKE) test

benchmark: all
	cd bm && $(MAKE) benchmark
