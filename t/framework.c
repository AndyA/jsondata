/* framework.c */

#include <stddef.h>
#include <stdlib.h>
#include <pthread.h>

#include "framework.h"
#include "tap.h"

#include "jd_pretty.h"

#define SIG 0x1A2B3C4D

struct memhdr {
  size_t size;
  struct memhdr *next;
  unsigned sig;
};

struct tls_data_struct {
  jd_activation *head;
  jd_var root_exception;
};

static size_t expected_leak_count = 0, expected_leak_size = 0;
static struct memhdr *memlist = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define HDR(m) ((struct memhdr *) (m) - 1)
#define MEM(h) ((struct memhdr *) (h) + 1)

static void *t_alloc(size_t size) {
  struct memhdr *h = malloc(size + sizeof(struct memhdr));
  if (!h)return NULL;
  h->size = size;
  h->sig = SIG;
  pthread_mutex_lock(&mutex);
  h->next = memlist;
  memlist = h;
  pthread_mutex_unlock(&mutex);
  return MEM(h);
}

static struct memhdr *unhook(struct memhdr *list, struct memhdr *h) {
  if (list == h) return list->next;
  if (h->sig != SIG) die("Block %p trampled: %08x", h, h->sig);
  list->next = unhook(list->next, h);
  return list;
}

static void t_free(void *m) {
  if (m) {
    struct memhdr *h = HDR(m);
    pthread_mutex_lock(&mutex);
    memlist = unhook(memlist, h);
    pthread_mutex_unlock(&mutex);
    free(h);
  }
}

static size_t get_leaks(unsigned *count) {
  struct memhdr *h;
  size_t sz = 0;
  *count = 0;

  for (h = memlist; h; h = h->next) {
    if (h->sig != SIG)
      die("Bad block %p (%08x)", h, h->sig);
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
  if (!ok(size == expected_leak_size && count == expected_leak_count, "memory leaks")) {
    diag("%lu bytes lost in %u allocations", (unsigned long) size, count);
  }
}

void expect_leak(size_t size) {
  expected_leak_count++;
  expected_leak_size += size;
}

int main(int argc, char *argv[]) {
  int i, count = 1;
  const char *tc_env = NULL;

  if (argc > 1)
    count = atoi(argv[1]);
  else if (tc_env = getenv("JD_TEST_COUNT"), tc_env)
    count = atoi(tc_env);
  else
    hook_alloc();

  set_vfpf(jd_vfprintf);

  expect_leak(sizeof(struct tls_data_struct));
  for (i = 0; i < count; i++)
    test_main();

  check_leaks();
  done_testing();
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
