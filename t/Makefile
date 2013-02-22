.PHONY: test valgrind

include ../common.mk

TESTS=$(wildcard *.t)
LIB=../libjsondata.a
TESTOBJ=$(TESTS:.t=.o)
TESTBIN=$(TESTS:.t=)
MISCOBJ=tap.o util.o jd_test.o
OBJ=$(TESTOBJ) $(MISCOBJ)
DEPS=$(OBJ:.o=.d) 

CFLAGS+=-I..
LDFLAGS+=-lpthread

all: $(TESTBIN)

%: %.o $(MISCOBJ) $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.t
	$(CC) -o $@ $(CFLAGS) -x c -c $<

%.d: %.c
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
	| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

%.d: %.t
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) -x c $< \
	| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
	[ -s $@ ] || rm -f $@'

include $(DEPS)

clean:
	rm -f $(OBJ) $(DEPS) $(TESTBIN)

test: all
	prove -e '' $(addprefix ./,$(TESTBIN))

valgrind: $(TESTBIN)
	prove -e 'valgrind -q' $(addprefix ./,$(TESTBIN))

