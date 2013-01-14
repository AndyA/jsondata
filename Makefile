.PHONY: all clean tags install version

PREFIX ?= /usr/local

CFLAGS=-Wall -O2 -D_LARGEFILE64_SOURCE
LDFLAGS=-lc -lm -lpthread

BINS=jdtest
BINOBJ=$(addsuffix .o,$(BINS))
MISCOBJ=jsondata.o jd_string.o jd_array.o
OBJS=$(BINOBJ) $(MISCOBJ)
DEPS=$(OBJS:.o=.d) 
INST_BINS=$(PREFIX)/bin

all: $(BINS)

version.h: VERSION
	perl tools/version.pl > version.h

%: %.o $(MISCOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.d: %.c version.h
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
	| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

include $(DEPS)

tags:
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .

clean:
	rm -f $(OBJS) $(DEPS) $(BINS) tags version.h

version:
	perl tools/bump_version.pl VERSION

install: $(BINS)
	touch VERSION
	$(MAKE)
	install -s -m 775 $(BINS) $(INST_BINS)
