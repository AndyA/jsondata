/* jd_path.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "jsondata.h"
#include "jd_private.h"
#include "jd_path.h"
#include "jd_pretty.h"

/* See:
 *  http://goessner.net/articles/JsonPath/
 *
 * Syntax is per that spec except that:
 *   (expr) and ?(expr) are (currently) unhandled
 *   ( | ) may be used as alternation
 *
 * The use of ( | ) to declare alternations doesn't clash with (expr)
 * because (expr) can only appear inside [] and alternation can only
 * appear outside []:
 *
 *         $.foo.(bar.0|baz.1[?(@.x < 3)])
 *               |      expr: ^^^^^^^^^^ |
 *  alternation: ^^^^^^^^^^^^^^^^^^^^^^^^^
 */

/* parsing */

void jd__path_init_parser(jd__path_parser *p, jd_var *path) {
  size_t len;
  p->path = path;
  p->sp = p->cp = jd_bytes(path, &len);
  p->ep = p->sp + len - 1;
}

static const char *single = "$@.*|,?[]()";

jd_var *jd__path_token(jd__path_parser *p) {
  if (p->cp == p->ep) return NULL;

  /* safe to go one char off the end: there's always a trailing nul */
  if (p->cp[0] == '.' && p->cp[1] == '.') {
    p->cp += 2;
    return jd_set_array_with(jd_nv(), jd_niv(JP_DOTDOT), NULL);
  }

  if (strchr(single, *(p->cp)))
    return jd_set_array_with(jd_nv(), jd_niv(*(p->cp)++), NULL);

  if (*(p->cp) == '\'') {
    JD_VAR(key);
    const char *sp = ++(p->cp);
    while (p->cp != p->ep && *(p->cp) != '\'')
      (p->cp)++;
    (p->cp)++;
    return jd_set_array_with(jd_nv(), jd_niv(JP_KEY),
                             jd_substr(key, p->path, sp - p->sp, p->cp - sp - 1),
                             NULL);
  }

  if (isdigit(*(p->cp))) {
    JD_VAR(idx);
    const char *sp = (p->cp)++;
    while (p->cp != p->ep && isdigit(*(p->cp)))
      (p->cp)++;
    return jd_set_array_with(jd_nv(), jd_niv(JP_INDEX),
                             jd_numify(idx, jd_substr(idx, p->path, sp - p->sp, p->cp - sp)),
                             NULL);
  }

  if (isalpha(*(p->cp))) {
    JD_VAR(key);
    const char *sp = (p->cp)++;
    while (p->cp != p->ep && isalnum(*(p->cp)))
      (p->cp)++;
    return jd_set_array_with(jd_nv(), jd_niv(JP_KEY),
                             jd_substr(key, p->path, sp - p->sp, p->cp - sp),
                             NULL);
  }

  jd_throw("Syntax error in path");
  return NULL;
}

static int if_literal_key(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_assign(result, context);
  jd_release(context);
  return 1;
}

/* context: the literal key
 * args:    a jd_var to iterate (ignored)
 * result:  a closure that will iterate all the keys
 */
static int pf_literal_key(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_set_closure(result, if_literal_key);
  jd_assign(jd_context(result), context);
  return 1;
}

/* Returns an array of closures. During iteration each closure will be called
 * with a pointer to part of a structure that is being iterated and will return
 * another closure, an iterator for all the keys, indexes or, in the case of '..',
 * sub-paths that match at the current location.
 */
static jd_var *path_parse(jd_var *out, jd__path_parser *p) {
  JD_2VARS(tok, cl);
  jd_set_array(out, 10);
  while (tok = jd__path_token(p), tok) {
    switch (jd_get_int(jd_get_idx(tok, 0))) {
    case '$':
    case '@':
    case '.':
      break;
    case JP_KEY:
    case JP_INDEX:
      jd_set_closure(cl, pf_literal_key);
      jd_assign(jd_context(cl), jd_get_idx(tok, 1));
      jd_assign(jd_push(out, 1), cl);
      break;
    default:
      jd_throw("Unhandled token: %J", tok);
    }
  }
  return out;
}

jd_var *jd__path_parse(jd_var *out, jd_var *path) {
  scope {
    jd__path_parser p;
    jd__path_init_parser(&p, path);
    path_parse(out, &p);
  }
  return out;
}

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

  scope {
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
  scope {
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
