/* util.c */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "util.h"

static unsigned long test_ops = 0;

static double now(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double) tv.tv_sec + (double) tv.tv_usec / 1000000;
}

void count_ops(unsigned long n) {
  test_ops += n;
}

int main(int argc, char *argv[]) {
  double max_time = 1.0;
  const char *tc_env = NULL;

  if (argc > 1)
    max_time = atof(argv[1]);
  else if (tc_env = getenv("JD_BM_TIME"), tc_env)
    max_time = atof(tc_env);

  setup();
  double end, start = now();
  while (end = now(), end < start + max_time)
    test();
  double elapsed = end - start;

  printf("%s: %lu operations, %.3f seconds, %.3f op/s, %.3f us/op\n",
         test_name, test_ops, elapsed,
         (double) test_ops / elapsed,
         1000000 * elapsed / (double) test_ops);

  teardown();

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
