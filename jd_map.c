/* jd_map.c */

#include "jd_private.h"
#include "jsondata.h"

typedef int (*filter_function)(jd_var *out, jd_var *cl, jd_var *in);

static jd_var *filter(jd_var *out, jd_var *cl, filter_function ff, jd_var *in);

static int map_filter(jd_var *out, jd_var *cl, jd_var *in) {
  jd_eval(cl, out, in);
  return 1;
}

static int grep_filter(jd_var *out, jd_var *cl, jd_var *in) {
  int ok;

  JD_BEGIN {
    JD_VAR(rv);
    jd_eval(cl, rv, in);
    ok = jd_test(rv);
    if (ok) jd_assign(out, in);
  }
  JD_END

  return ok;
}

static jd_var *filter_array(jd_var *out, jd_var *cl, filter_function ff, jd_var *in) {
  JD_BEGIN {
    size_t count = jd_count(in);
    JD_VAR(rv);
    unsigned i;

    jd_set_array(out, count);
    for (i = 0; i < count; i++) {
      jd_var *v = jd_get_idx(in, i);
      if (JD_IS_SIMPLE(v->type)) {
        if (ff(rv, cl, v)) jd_assign(jd_push(out, 1), rv);
        jd_release(rv);
      }
      else {
        filter(jd_push(out, 1), cl, ff, v);
      }
    }
  }
  JD_END

  return out;
}

static jd_var *filter(jd_var *out, jd_var *cl, filter_function ff, jd_var *in) {
  switch (in->type) {
  case ARRAY:
    return filter_array(out, cl, ff, in);
  default:
    jd_throw("Can't handle type");
  }
  return NULL;
}

jd_var *jd_map(jd_var *out, jd_var *func, jd_var *in) {
  return filter(out, func, map_filter, in);
}

jd_var *jd_grep(jd_var *out, jd_var *func, jd_var *in) {
  return filter(out, func, grep_filter, in);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
