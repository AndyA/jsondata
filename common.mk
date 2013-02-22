CFLAGS=-Wall -Werror -D_LARGEFILE64_SOURCE
LDFLAGS=-lc -lm
ifneq ($(shell $(CC) -v 2>&1 | grep -i clang),)
CFLAGS+=-Qunused-arguments
endif
ifneq ($(PROFILE),)
CFLAGS+=-fprofile-arcs -ftest-coverage
LDFLAGS+=-lgcov
endif
ifneq ($(DEBUG),)
CFLAGS+=-g2
else
CFLAGS+=-O3
endif
CFLAGS+=-pthread
LDFLAGS+=-pthread
