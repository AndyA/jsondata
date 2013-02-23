/* jd_path.c */

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
  jds = jd_as_string(v);
  sl = jd_string_length(jds);
  if (sl == 0) return 0;
  if (jds->data[0] == '0' && sl != 1) return 0;
  for (i = 0; i < sl; i++)
    if (!isdigit(jds->data[i])) return 0;
  return 1;
}

jd_var *jd_get_context(jd_var *root, jd_var *path,
                       jd_path_context *ctx, int vivify) {
  unsigned depth;
  jd_var *ptr;

  JD_BEGIN {
    JD_3VARS(part, wrap, elt);

    if (path->type == ARRAY) {
      jd_assign(part, path);
    }
    else {
      JD_SV(dot, ".");
      jd_split(part, path, dot);
    }

    /* move root inside a hash: { "$": root } */
    jd_set_hash(wrap, 1);
    jd_assign(jd_get_ks(wrap, "$", 1), root);

    for (ptr = wrap, depth = 0; ptr && jd_shift(part, 1, elt); depth++) {
      /* TODO nasty special case to make sure we target the
       * variable we're building into rather than our own
       * copy as we step out of the synthetic root hash and
       * into the real thing.
       */
      jd_var *targ = depth == 1 ? root : ptr;
      if (targ->type == VOID) {
        /* empty slot: type depends on key format */
        if (is_positive_int(elt))
          jd_set_array(targ, 1);
        else
          jd_set_hash(targ, 1);
      }

      if (targ->type == ARRAY) {
        size_t ac = jd_count(targ);
        jd_int ix = jd_get_int(elt);
        if (ix == ac && vivify)
          ptr = jd_push(targ, 1);
        else if (ix < ac)
          ptr = jd_get_idx(targ, ix);
        else {
          ptr = NULL;
          break;
        }
      }
      else if (targ->type == HASH) {
        ptr = jd_get_key(targ, elt, vivify && depth > 0);
        if (!ptr) {
          if (!vivify) JD_RETURN(NULL);
          jd_throw("Bad path");
        }
      }
      else {
        jd_throw("Unexpected element in structure");
      }
    }
  } JD_END

  /* Hack: depth 0 means empty path, depth 1 means root - so don't
   * return a pointer to the innards of wrap. Anything else is inside
   * the structure and safe to return directly.
   */
  switch (depth) {
  case 0:
    return NULL;
  case 1:
    return root;
  default:
    return ptr;
  }
}

static jd_var *getter(jd_var *root, const char *path, va_list ap, int vivify) {
  jd_var *rv = NULL;
  JD_BEGIN {
    JD_VAR(pv);
    jd_vprintf(pv, path, ap);
    rv = jd_get_context(root, pv, NULL, vivify);
  } JD_END
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
