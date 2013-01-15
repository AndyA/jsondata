.PHONY: all clean tags install version test

include common.mk

PREFIX ?= /usr/local

BINS=jdtest
BINOBJ=$(addsuffix .o,$(BINS))
LIB=libjsondata.a
LIBOBJ=jsondata.o jd_string.o jd_array.o
OBJS=$(BINOBJ) $(LIBOBJ)
DEPS=$(OBJS:.o=.d) 
INST_BINS=$(PREFIX)/bin

all: $(LIB) $(BINS)

version.h: VERSION
	perl tools/version.pl > version.h

%: %.o $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS)

$(LIB): $(LIBOBJ)
	ar rcs $@ $^

%.d: %.c version.h
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
	| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

include $(DEPS)

tags:
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .

clean:
	rm -f $(OBJS) $(DEPS) $(BINS) tags version.h
	cd t && $(MAKE) clean

version:
	perl tools/bump_version.pl VERSION

test: $(LIB)
	cd t && $(MAKE) test

install: $(BINS)
	touch VERSION
	$(MAKE)
	install -s -m 775 $(BINS) $(INST_BINS)
