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

static int if_list(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_var *ctx = jd_get_idx(context, 0);
  jd_int cnt = (jd_int) jd_count(&ctx[0]);
  jd_int idx = jd_get_int(&ctx[1]);
  if (idx < cnt) {
    jd_assign(result, jd_get_idx(&ctx[0], idx));
    jd_set_int(&ctx[1], idx + 1);
  }
  else {
    jd_set_void(result);
  }
  return 1;
}

static int if_seq(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_int idx = jd_get_int(jd_get_idx(context, 0));
  jd_int lim = jd_get_int(jd_get_idx(context, 1));
  if (idx < lim) {
    jd_assign(result, jd_get_idx(context, 0));
    jd_set_int(jd_get_idx(context, 0), idx + 1);
  }
  else {
    jd_set_void(result);
  }
  return 1;
}

static void list_iter(jd_var *out, jd_var *list) {
  jd_set_closure(out, if_list);
  jd_var *slot = jd_push(jd_set_array(jd_context(out), 2), 2);
  jd_assign(slot++, list);
  jd_set_int(slot++, 0);
}

static void seq_iter(jd_var *out, jd_int idx, jd_int lim) {
  jd_set_closure(out, if_seq);
  jd_var *slot = jd_push(jd_set_array(jd_context(out), 2), 2);
  jd_set_int(slot++, idx);
  jd_set_int(slot++, lim);
}

static int pf_wild(jd_var *result, jd_var *context, jd_var *args) {
  (void) context;
  if (args && args->type == HASH) {
    jd_var keys = JD_INIT;
    list_iter(result, jd_keys(&keys, args));
    jd_release(&keys);
    return 1;
  }
  if (args && args->type == ARRAY) {
    seq_iter(result, 0, (jd_int) jd_count(args));
    return 1;
  }
  seq_iter(result, 0, 0); /* empty */
  return 1;
}

static int pf_list(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  list_iter(result, context);
  return 1;
}

static int if_literal(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_assign(result, context);
  jd_release(context);
  return 1;
}

/* context: the literal key
 * args:    a jd_var to iterate (ignored)
 * result:  a closure that will iterate all the keys
 */
static int pf_literal(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_set_closure(result, if_literal);
  jd_assign(jd_context(result), context);
  return 1;
}

/* Returns an array of closures. During iteration each closure will be called
 * with a pointer to part of a structure that is being iterated and will return
 * another closure: an iterator for all the keys, indexes or, in the case of '..',
 * sub-paths that match at the current location.
 */
static jd_var *path_parse(jd_var *out, jd__path_parser *p) {
  JD_2VARS(tok, cl);
  jd_set_array(out, 10);
  while (tok = jd__path_token(p), tok) {
    jd_set_void(cl);
    switch (jd_get_int(jd_get_idx(tok, 0))) {
    case '$':
    case '@':
    case '.':
      break;
    case JP_KEY:
    case JP_INDEX:
      jd_set_closure(cl, pf_literal);
      jd_assign(jd_context(cl), jd_get_idx(tok, 1));
      break;
    case '*':
      jd_set_closure(cl, pf_wild);
      break;
    case '#': /* TEMPORARY, BROKEN (avoid compiler warning for pf_list) */
      jd_set_closure(cl, pf_list);
      break;
    default:
      jd_throw("Unhandled token: %J", tok);
    }
    if (cl->type != VOID) jd_assign(jd_push(out, 1), cl);
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

jd_var *jd__path_compile(jd_var *path) {
  jd_var *magic = jd__get_magic(path, MAGIC_PATH);
  if (magic->type == VOID)
    jd__path_parse(magic, path);
  return magic;
}

static int iter_maker_iter(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_var *ctx = jd_get_idx(context, 0); /* array */

  jd_var *makers = ctx++;
  jd_var *idx = ctx++;
  jd_var *current = ctx++;

  for (;;) {
    while (current->type == VOID) {
      if (jd_get_int(idx) == (jd_int) jd_count(makers)) {
        jd_set_void(result);
        return 1;
      }

      jd_eval(jd_get_idx(makers, jd_get_int(idx)), current, NULL);
      jd_set_int(idx, jd_get_int(idx) + 1);
    }
    jd_eval(current, result, NULL);
    if (result->type != VOID)
      return 1;
    jd_set_void(current);
  }
}

/* That really is the right name... */
jd_var *jd__make_iter_maker_iter(jd_var *out, jd_var *makers) {
  jd_var *ctx = jd_set_array(jd_context(jd_set_closure(out, iter_maker_iter)), 10);
  jd_var *slot = jd_push(ctx, 3);
  jd_assign(slot++, makers);
  jd_set_int(slot++, 0); /* index into makers */
  jd_set_void(slot++); /* redundant init: empty slot for current iter */

  return out;
}

static int iter_func(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  scope {
    jd_var *ctx = jd_get_idx(context, 0); /* array */
    jd_var *path = ctx++;
    jd_var *var = ctx++;
    jd_var *iters = ctx++;
    jd_var *active = ctx++;
    jd_var *vivify = ctx++;
    jd_int ipos;

    (void) var;
    (void) vivify;

    while (jd_count(active) < jd_count(path)) {
      jd_var nv = JD_INIT;

      while (ipos = (jd_int) jd_count(iters), ipos <= (jd_int) jd_count(active))
        jd_eval(jd_get_idx(path, ipos), jd_push(iters, 1), NULL);

      /*jd__make_iter_maker_iter(jd_push(iters, 1), jd_get_idx(path, ipos));*/

      jd_eval(jd_get_idx(iters, ipos - 1), &nv, NULL);
      if (nv.type == VOID) {
        /* don't need to release nv if it's void */
        if (jd_count(active) == 0) {
          jd_set_void(result);
          JD_RETURN(1);
        }
        jd_pop(active, 1, NULL);
        jd_pop(iters, 1, NULL);
        continue;
      }

      jd_assign(jd_push(active, 1), &nv);
      jd_release(&nv);
    }

    /* return [ slot, path, captures ] */
    jd_set_array(result, 2);
    jd_var *slot = jd_push(result, 2);
    jd_set_void(slot++); /* TODO slot */
    jd_join(slot++, jd_nsv("."), active); /* path */
    jd_pop(active, 1, NULL);
  }

  return 1;
}

jd_var *jd_path_iter(jd_var *out, jd_var *v, jd_var *path, int vivify) {
  jd_var *ctx = jd_set_array(jd_context(jd_set_closure(out, iter_func)), 10);
  /* build context */
  jd_var *slot = jd_push(ctx, 5);
  jd_assign(slot++, jd__path_compile(path));
  jd_assign(slot++, v);
  jd_set_array(slot++, 10); /* active iterators */
  jd_set_array(slot++, 10); /* current path */
  jd_set_bool(slot++, vivify);
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
