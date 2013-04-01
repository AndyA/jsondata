TESTBIN = jsonpath

EXTRA_DIST = model.json

noinst_PROGRAMS = $(TESTBIN)

noinst_LTLIBRARIES = libbm-support.la

libbm_support_la_SOURCES = 
libbm_support_la_LDFLAGS = -static

LDADD = libbm-support.la ../libjsondata.la

test: $(noinst_PROGRAMS)
	prove -e '' $(addprefix ./,$(noinst_PROGRAMS))
