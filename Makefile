.PHONY: all clean tags install version test valgrind

include common.mk

PREFIX ?= /usr/local

BINS=jdtest
BINOBJ=$(addsuffix .o,$(BINS))
LIB=libjsondata.a
LIBOBJ=jsondata.o jd_string.o jd_array.o jd_hash.o jd_path.o jd_json.o
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
	rm -f $(OBJS) $(DEPS) $(BINS) $(LIB) tags version.h
	$(MAKE) -C t clean

version:
	perl tools/bump_version.pl VERSION

test: $(LIB)
	$(MAKE) -C t test

valgrind: $(LIB)
	$(MAKE) -C t valgrind

install: $(BINS)
	touch VERSION
	$(MAKE)
	install -s -m 775 $(BINS) $(INST_BINS)
