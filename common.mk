CFLAGS=-Wall -Werror -g2 -D_LARGEFILE64_SOURCE
ifneq ($(shell $(CC) -v 2>&1 | grep -i clang),)
CFLAGS+=-Qunused-arguments
endif
LDFLAGS=-lc -lm

