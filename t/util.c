/* util.c */

#include <stddef.h>
#include <stdlib.h>

#include "util.h"
#include "tap.h"

#include "jsondata.h"

struct memhdr {
  size_t size;
  struct memhdr *next;
};

struct memhdr *memlist = NULL;

#define HDR(m) ((struct memhdr *) (m) - 1)
#define MEM(h) ((struct memhdr *) (h) + 1)

static void *t_alloc(size_t size) {
  struct memhdr *h = malloc(size + sizeof(struct memhdr));
  if (!h)return NULL;
  h->size = size;
  h->next = memlist;
  memlist = h;
  return MEM(h);
}

static struct memhdr *unhook(struct memhdr *list, struct memhdr *h) {
  if (list == h) return list->next;
  list->next = unhook(list->next, h);
  return list;
}

static void t_free(void *m) {
  if (m) {
    struct memhdr *h = HDR(m);
    memlist = unhook(memlist, h);
    free(h);
  }
}

static size_t get_leaks(unsigned *count) {
  struct memhdr *h;
  size_t sz = 0;
  *count = 0;

  for (h = memlist; h; h = h->next) {
    sz += h->size;
    (*count)++;
  }

  return sz;
}


static void hook_alloc(void) {
  jd_alloc_hook = t_alloc;
  jd_free_hook = t_free;
}

static void check_leaks(void) {
  size_t size;
  unsigned count;
  size = get_leaks(&count);
  if (!ok(size == 0 && count == 0, "memory leaks")) {
    diag("%lu bytes lost in %u allocations", (unsigned long) size, count);
  }
}

void test_init(void) {
  hook_alloc();
}

void test_done(void) {
  check_leaks();
  done_testing();
}

int main(void) {
  test_init();
  test_main();
  test_done();
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
