/* thread.t */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

#define NTHREAD 10

static void nsleep(double s) {
  struct timespec ts;
  ts.tv_sec = (time_t) s;
  ts.tv_nsec = (long)((s - (time_t) s) * 1000000000);
  nanosleep(&ts, NULL);
}

static void do_stuff(jd_var *v, int depth) {
  int i;

  static const char *kn[] = {
    "one",
    "two",
    "three",
    NULL
  };

  nsleep(0.001);

  JD_SCOPE {
    switch (depth) {
    case 0:
    case 2:
      jd_set_array(v, 10);
      for (i = 0; i < 3; i++)
        do_stuff(jd_push(v, 1), depth + 1);
      break;
    case 1:
    case 3:
      jd_set_hash(v, 10);
      for (i = 0; kn[i]; i++)
        do_stuff(jd_lv(v, "$.%s", kn[i]), depth + 1);
      break;
    default:
      jd_printf(v, "At depth %d", depth);
      break;
    }
  }
}

static void *test_threads(void *tid) {
  jd_var *slot = (jd_var *) tid;
  do_stuff(slot, 0);
  return NULL;
}

void test_main(void) {
  JD_SCOPE {
    int i;
    pthread_t thd[NTHREAD];
    JD_AV(ar, NTHREAD);

    for (i = 0; i < NTHREAD; i++) {
      jd_var *slot = jd_push(ar, 1);
      pthread_create(&thd[i], NULL, test_threads, slot);
    }

    for (i = 0; i < NTHREAD; i++) {
      void *rc;
      pthread_join(thd[i], &rc);
    }

    for (i = 1; i < NTHREAD; i++) {
      jdt_is(jd_get_idx(ar, 0), jd_get_idx(ar, i), "slot %d matches slot 0", i);
    }
  }
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
