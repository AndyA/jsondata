.PHONY: test

include ../common.mk

TESTS=$(wildcard *.t)
LIB=../libjsondata.a
TESTOBJ=$(TESTS:.t=.o)
TESTBIN=$(TESTS:.t=)
MISCOBJ=tap.o util.o
OBJ=$(TESTOBJ) $(MISCOBJ)
DEPS=$(OBJ:.o=.d) 

CFLAGS+=-I..

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

test: $(TESTBIN)
	prove -e '' $(addprefix ./,$(TESTBIN))
