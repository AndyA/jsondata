TESTBIN = 00misc array clone closure compare exception exit get \
	  hash json jsonpath magic map model object path string \
	  thread utf8

EXTRA_DIST = model.json jsonpath.json walk.json

noinst_PROGRAMS = $(TESTBIN)

noinst_LTLIBRARIES = libtest-support.la

libtest_support_la_SOURCES = tap.c tap.h framework.c framework.h jd_test.c jd_test.h
libtest_support_la_LDFLAGS = -static

LDADD = libtest-support.la ../libjsondata.la

test: $(noinst_PROGRAMS)
	prove -e '' $(addprefix ./,$(noinst_PROGRAMS))

valgrind: $(noinst_PROGRAMS)
	prove -e 'valgrind -q' $(addprefix ./,$(noinst_PROGRAMS))
