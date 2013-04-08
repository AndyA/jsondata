DIST_SUBDIRS = bm t m4

EXTRA_DIST = jd_path.h jd_utf8.h

SUBDIRS = . bm t

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libjsondata.la

include_HEADERS = jsondata.h jd_pretty.h

noinst_PROGRAMS = jdtest

bin_PROGRAMS = jsonpretty

libjsondata_la_SOURCES = jsondata.h jd_private.h \
	jsondata.c jd_string.c jd_array.c jd_hash.c \
	jd_path.c jd_json.c jd_closure.c jd_object.c \
	jd_printf.c jd_exception.c jd_map.c jd_magic.c \
	jd_utf8.c

libjsondata_la_LDFLAGS = -avoid-version

jdtest_SOURCES = jdtest.c

jdtest_LDADD = libjsondata.la

jsonpretty_SOURCES = jsonpretty.c

jsonpretty_LDADD = libjsondata.la

test: all
	cd t && $(MAKE) test

benchmark: all
	cd bm && $(MAKE) benchmark

PKGBUILDDIR = $(abs_builddir)/.build
PKGDEBDIR = $(PKGBUILDDIR)/deb
PKGRPMDIR = $(PKGBUILDDIR)/rpm

clean-local:
	-rm -rf $(PKGBUILDDIR)


deb: dist
	rm -rf $(PKGDEBDIR)
	mkdir -p $(PKGDEBDIR)
	cp $(PACKAGE)-$(VERSION).tar.gz $(PKGDEBDIR)/
	tar --directory $(PKGDEBDIR)/ -xzf $(PKGDEBDIR)/$(PACKAGE)-$(VERSION).tar.gz
	cd $(PKGDEBDIR)/$(PACKAGE)-$(VERSION)/; echo | \
		dh_make --single --copyright gpl -e andy@hexten.net \
	       	-f ../$(PACKAGE)-$(VERSION).tar.gz
	cp packaging/deb/changelog packaging/deb/control packaging/deb/copyright \
		$(PKGDEBDIR)/$(PACKAGE)-$(VERSION)/debian/
	cd $(PKGDEBDIR)/$(PACKAGE)-$(VERSION)/; \
		rm -f debian/README.Debian debian/*.ex debian/*.EX; \
		./configure --prefix=/usr; \
		dpkg-buildpackage -rfakeroot; \
		mv ../*.deb .

rpm: dist
	rm -rf $(PKGRPMDIR)/BUILD/$(PACKAGE)-root
	rm -rf $(PKGRPMDIR)/RPMS/$(PACKAGE)-*
	rm -rf $(PKGRPMDIR)/SRPMS/$(PACKAGE)-*
	rm -rf $(PKGRPMDIR)/SOURCES/$(PACKAGE)-*
	mkdir -p $(PKGRPMDIR)/RPMS/i386
	mkdir -p $(PKGRPMDIR)/RPMS/x86_64
	mkdir -p $(PKGRPMDIR)/SRPMS
	mkdir -p $(PKGRPMDIR)/BUILD
	mkdir -p $(PKGRPMDIR)/SOURCES
	mkdir -p $(PKGRPMDIR)/tmp
	cp $(PACKAGE)-$(VERSION).tar.gz $(PKGRPMDIR)/SOURCES/
	rpmbuild --quiet --nodeps --define="_topdir $(PKGRPMDIR)" -ba packaging/jsondata.spec
	mv $(PKGRPMDIR)/RPMS/x86_64/$(PACKAGE)-$(VERSION)-*.x86_64.rpm .
