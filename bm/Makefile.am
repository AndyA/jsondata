TESTBIN = jsonpath jsonpath-cache-compile

EXTRA_DIST = model.json benchmark.sh

noinst_PROGRAMS = $(TESTBIN)

noinst_LTLIBRARIES = libbm-support.la

libbm_support_la_SOURCES = util.c util.h
libbm_support_la_LDFLAGS = -static

LDADD = libbm-support.la ../libjsondata.la

benchmark: $(noinst_PROGRAMS)
	bash benchmark.sh $(noinst_PROGRAMS)
