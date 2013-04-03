/* jsonpath.c */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "util.h"
#include "jd_pretty.h"

static jd_var data = JD_INIT;
static jd_var data2 = JD_INIT;
static jd_var paths = JD_INIT;

const char *test_name = "jsonpath";

static int path_iter_f(jd_var *result, jd_var *context, jd_var *arg) {
  jd_var *var = jd_get_idx(context, 0);
  jd_var *stack = jd_get_idx(context, 1);

  (void) arg;

  jd_set_void(result);
  if (jd_count(stack) == 0) return 1;

  scope {
    jd_var *slot = jd_nv();
    int i;
    size_t cnt;

    jd_pop(stack, 1, result);
    slot = jd_rv(var, jd_bytes(result, NULL));
    if (!slot) jd_die("Can't find %s", jd_bytes(result, NULL));

    if (slot->type == ARRAY) {
      cnt = jd_count(slot);
      for (i = cnt - 1; i >= 0; i--)
        jd_sprintf(jd_push(stack, 1), "%V.%u", result, i);
    }
    else if (slot->type == HASH) {
      jd_var *keys = jd_keys(jd_nv(), slot);
      cnt = jd_count(keys);
      for (i = cnt - 1; i >= 0; i--)
        jd_sprintf(jd_push(stack, 1), "%V.%V", result, jd_get_idx(keys, i));
    }
  }

  return 1;
}

static jd_var *path_iter(jd_var *iter, jd_var *data) {
  jd_var *slot = jd_push(jd_set_array(jd_context(jd_set_closure(iter, path_iter_f)), 2), 2);
  jd_assign(slot++, data);
  jd_set_string(jd_push(jd_set_array(slot++, 1000), 1), "$");
  return iter;
}

void setup(void) {
  scope {
    load_json(&data, "model.json");

    jd_set_array(&paths, 1000);
    jd_var *iter = path_iter(jd_nv(), &data);
    jd_var *path = jd_nv();
    for (;;) {
      jd_eval(iter, path, NULL);
      if (path->type == VOID) break;
      jd_assign(jd_push(&paths, 1), path);
    }
  }
}

void test(void) {
  unsigned i;
  size_t pcnt = jd_count(&paths);

  jd_set_void(&data2);
  for (i = 0; i < pcnt; i++) {
    jd_var *path = jd_get_idx(&paths, i);
    const char *pstr = jd_bytes(path, NULL);
    jd_assign(jd_lv(&data2, pstr), jd_rv(&data, pstr));
    count_ops(1);
  }

  jd_assign(&data, &data2);
}

void teardown(void) {
  jd_release(&data);
  jd_release(&data2);
  jd_release(&paths);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
