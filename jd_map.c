/* jd_map.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jd_private.h"
#include "jsondata.h"

typedef int (*filter_function)(jd_var *out, jd_var *cl, jd_var *in);

static jd_var *filter(jd_var *out, jd_var *cl, filter_function ff,
                      int flat, jd_var *in);

#define IS_FILTERED(t) (!JD_IS_COMPLEX(t))

static int map_filter(jd_var *out, jd_var *cl, jd_var *in) {
  jd_eval(cl, out, in);
  return 1;
}

static int grep_filter(jd_var *out, jd_var *cl, jd_var *in) {
  int ok = 0;

  JD_SCOPE {
    JD_VAR(rv);
    jd_eval(cl, rv, in);
    ok = jd_test(rv);
    if (ok) jd_assign(out, in);
  }

  return ok;
}

static jd_var *filter_array(jd_var *out, jd_var *cl, filter_function ff,
                            int flat, jd_var *in) {
  JD_SCOPE {
    size_t count = jd_count(in);
    JD_VAR(rv);
    unsigned i;

    jd_set_array(out, count);
    for (i = 0; i < count; i++) {
      jd_var *v = jd_get_idx(in, i);
      if (flat || IS_FILTERED(v->type)) {
        if (jd_set_void(rv), ff(rv, cl, v)) jd_assign(jd_push(out, 1), rv);
      }
      else {
        filter(jd_push(out, 1), cl, ff, flat, v);
      }
    }
  }

  return out;
}

static jd_var *filter_hash(jd_var *out, jd_var *cl, filter_function ff,
                           int flat, jd_var *in) {
  JD_SCOPE {
    size_t count = jd_count(in);
    JD_2VARS(rv, keys);
    unsigned i;

    jd_keys(keys, in);
    jd_set_hash(out, count);
    for (i = 0; i < count; i++) {
      jd_var *k = jd_get_idx(keys, i);
      jd_var *v = jd_get_key(in, k, 0);
      if (flat || IS_FILTERED(v->type)) {
        if (jd_set_void(rv), ff(rv, cl, v)) jd_assign(jd_get_key(out, k, 1), rv);
      }
      else {
        filter(jd_get_key(out, k, 1), cl, ff, flat, v);
      }
    }
  }

  return out;
}

static jd_var *filter_scalar(jd_var *out, jd_var *cl, filter_function ff,
                             int flat, jd_var *in) {
  (void) flat;
  JD_SCOPE {
    JD_VAR(rv);
    if (ff(rv, cl, in)) jd_assign(out, rv);
    else jd_set_void(out);
  }
  return out;
}

static jd_var *filter(jd_var *out, jd_var *cl, filter_function ff,
                      int flat, jd_var *in) {
  switch (in->type) {
  case ARRAY:
    return filter_array(out, cl, ff, flat, in);
  case HASH:
    return filter_hash(out, cl, ff, flat, in);
  default:
    return filter_scalar(out, cl, ff, flat, in);
  }
}

static jd_var *safe_filter(jd_var *out, jd_var *cl, filter_function ff,
                           int flat, jd_var *in) {
  JD_SCOPE {
    JD_VAR(tmp);
    filter(tmp, cl, ff, flat, in);
    jd_assign(out, tmp);
  }
  return out;
}

jd_var *jd_map(jd_var *out, jd_var *func, jd_var *in) {
  return safe_filter(out, func, map_filter, 1, in);
}

jd_var *jd_grep(jd_var *out, jd_var *func, jd_var *in) {
  return safe_filter(out, func, grep_filter, 1, in);
}

jd_var *jd_dmap(jd_var *out, jd_var *func, jd_var *in) {
  return safe_filter(out, func, map_filter, 0, in);
}

jd_var *jd_dgrep(jd_var *out, jd_var *func, jd_var *in) {
  return safe_filter(out, func, grep_filter, 0, in);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
