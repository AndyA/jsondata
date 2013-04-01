/* jsonpath.c */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "jd_pretty.h"

static jd_var data = JD_INIT;
static jd_var data2 = JD_INIT;
static jd_var paths = JD_INIT;
static unsigned long ops = 0;

static jd_var *load_string(jd_var *out, const char *filename) {
  FILE *f;
  char buf[512];
  size_t got;

  if (f = fopen(filename, "r"), !f) jd_die("Can't read %s", filename);
  jd_set_empty_string(out, 100);
  while (got = fread(buf, 1, sizeof(buf), f), got) {
    jd_append_bytes(out, buf, got);
  }
  fclose(f);
  return out;
}

static jd_var *load_json(jd_var *out, const char *filename) {
  jd_var json = JD_INIT;
  jd_from_json(out, load_string(&json, filename));
  jd_release(&json);
  return out;
}

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
        jd_printf(jd_push(stack, 1), "%V.%u", result, i);
    }
    else if (slot->type == HASH) {
      jd_var *keys = jd_keys(jd_nv(), slot);
      cnt = jd_count(keys);
      for (i = cnt - 1; i >= 0; i--)
        jd_printf(jd_push(stack, 1), "%V.%V", result, jd_get_idx(keys, i));
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

static void setup(void) {
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

static double now(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double) tv.tv_sec + (double) tv.tv_usec / 1000000;
}

static void test(void) {
  unsigned i;
  size_t pcnt = jd_count(&paths);

  jd_set_void(&data2);
  for (i = 0; i < pcnt; i++) {
    jd_var *path = jd_get_idx(&paths, i);
    const char *pstr = jd_bytes(path, NULL);
    jd_assign(jd_lv(&data2, pstr), jd_rv(&data, pstr));
    ops++;
  }

  jd_assign(&data, &data2);
}

static void pdata(jd_var *data) {
  scope {
    printf("%s\n", jd_bytes(jd_to_json_pretty(jd_nv(), data), NULL));
  }
}

int main(int argc, char *argv[]) {
  int i, count = 1;
  const char *tc_env = NULL;

  if (argc > 1)
    count = atoi(argv[1]);
  else if (tc_env = getenv("JD_BM_COUNT"), tc_env)
    count = atoi(tc_env);

  setup();
  double start = now();
  for (i = 0; i < count; i++)
    test();
  double elapsed = now() - start;

  if (count == 1)
    pdata(&data);

  printf("%lu operations, %.2f seconds, %.2f op/s, %.2f us/op\n",
         ops, elapsed, (double) ops / elapsed, 1000000 * elapsed / (double) ops);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
