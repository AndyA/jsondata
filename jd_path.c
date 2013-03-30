/* jd_path.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "jd_private.h"
#include "jsondata.h"

static int is_positive_int(jd_var *v) {
  jd_string *jds;
  size_t sl;
  unsigned i;

  if (v->type != STRING) return 1;
  jds = jd__as_string(v);
  sl = jd__string_length(jds);
  if (sl == 0) return 0;
  if (jds->data[0] == '0' && sl != 1) return 0;
  for (i = 0; i < sl; i++)
    if (!isdigit(jds->data[i])) return 0;
  return 1;
}

jd_var *jd_get_context(jd_var *root, jd_var *path,
                       jd_path_context *ctx, int vivify) {
  jd_var *ptr = NULL;
  (void) ctx;

  JD_SCOPE {
    JD_2VARS(part, elt);

    if (path->type == ARRAY)
      jd_clone(part, path, 0);
    else
      jd_split(part, path, jd_nsv("."));

    if (!jd_shift(part, 1, elt) || jd_compare(elt, jd_nsv("$")))
      jd_throw("Bad path");

    for (ptr = root; ptr && jd_shift(part, 1, elt);) {
      if (ptr->type == VOID) {
        /* empty slot: type depends on key format */
        if (is_positive_int(elt))
          jd_set_array(ptr, 1);
        else
          jd_set_hash(ptr, 1);
      }

      if (ptr->type == ARRAY) {
        int  ac = (int) jd_count(ptr);
        jd_int ix = jd_get_int(elt);
        if (ix == ac && vivify)
          ptr = jd_push(ptr, 1);
        else if (ix < ac)
          ptr = jd_get_idx(ptr, ix);
        else {
          ptr = NULL;
        }
      }
      else if (ptr->type == HASH) {
        ptr = jd_get_key(ptr, elt, vivify);
      }
      else {
        jd_throw("Unexpected element in structure");
      }
    }
  }

  return ptr;
}

static jd_var *getter(jd_var *root, const char *path, va_list ap, int vivify) {
  jd_var *rv = NULL;
  JD_SCOPE {
    JD_VAR(pv);
    jd_vprintf(pv, path, ap);
    rv = jd_get_context(root, pv, NULL, vivify);
  }
  return rv;
}

jd_var *jd_lv(jd_var *root, const char *path, ...) {
  jd_var *rv;
  va_list ap;
  va_start(ap, path);
  rv = getter(root, path, ap, 1);
  va_end(ap);
  return rv;
}

jd_var *jd_rv(jd_var *root, const char *path, ...) {
  jd_var *rv;
  va_list ap;
  va_start(ap, path);
  rv = getter(root, path, ap, 0);
  va_end(ap);
  return rv;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
