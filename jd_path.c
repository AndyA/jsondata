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

/* === Iterator stuff === */

/* == Primitives == */

/* = A list of items = */

static int iterate_list(jd_var *result, jd_var *context, jd_var *args) {
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

static jd_var *make_list_iter(jd_var *out, jd_var *list) {
  jd_set_closure(out, iterate_list);
  jd_var *slot = jd_push(jd_set_array(jd_context(out), 2), 2);
  jd_assign(slot++, list);
  jd_set_int(slot++, 0);
  return out;
}

/* = A start / end / step slice = */

static int iterate_slice(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_int idx = jd_get_int(jd_get_idx(context, 0));
  jd_int lim = jd_get_int(jd_get_idx(context, 1));
  jd_int stp = jd_get_int(jd_get_idx(context, 2));
  if (idx < lim) {
    jd_assign(result, jd_get_idx(context, 0));
    jd_set_int(jd_get_idx(context, 0), idx + stp);
  }
  else {
    jd_set_void(result);
  }
  return 1;
}

static jd_var *make_slice(jd_var *out, jd_int idx, jd_int lim, jd_int stp) {
  jd_set_closure(out, iterate_slice);
  jd_var *slot = jd_push(jd_set_array(jd_context(out), 3), 3);
  jd_set_int(slot++, idx);
  jd_set_int(slot++, lim);
  jd_set_int(slot++, stp);
  return out;
}

/* == Factories == */

/* = A literal = */

static int iterate_literal(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_assign(result, context);
  jd_release(context);
  return 1;
}

static int spawn_literal(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_assign(jd_context(jd_set_closure(result, iterate_literal)), context);
  return 1;
}

static jd_var *make_literal_factory(jd_var *out, jd_var *lit) {
  jd_assign(jd_context(jd_set_closure(out, spawn_literal)), lit);
  return out;
}

/* = A wildcard = */

static int spawn_wild(jd_var *result, jd_var *context, jd_var *args) {
  (void) context;
  if (args && args->type == HASH) {
    jd_var keys = JD_INIT;
    jd_keys(&keys, args);
    jd_sort(&keys);
    make_list_iter(result, &keys);
    jd_release(&keys);
    return 1;
  }
  if (args && args->type == ARRAY) {
    make_slice(result, 0, (jd_int) jd_count(args), 1);
    return 1;
  }
  make_slice(result, 0, 0, 1); /* empty */
  return 1;
}

static jd_var *make_wild_factory(jd_var *out) {
  jd_set_closure(out, spawn_wild);
  return out;
}

/* = A slice = */

static int spawn_slice(jd_var *result, jd_var *context, jd_var *args) {
  size_t tsz = jd_count(context);

  jd_int vivify = jd_get_int(result);
  jd_int start = jd_get_int(jd_get_idx(context, 1));
  jd_int end = tsz > 2 ? jd_get_int(jd_get_idx(context, 2)) : start + 1;
  jd_int step = tsz > 3 ? jd_get_int(jd_get_idx(context, 3)) : 1;

  if (!vivify && args && args->type == ARRAY) {
    jd_int sz = (jd_int) jd_count(args);
    if (end > sz) end = sz;
  }

  make_slice(result, start, end, step);

  return 1;
}

static jd_var *make_slice_factory(jd_var *out, jd_var *tok) {
  if (jd_count(tok) == 2)
    make_literal_factory(out, jd_get_idx(tok, 1));
  else
    jd_assign(jd_context(jd_set_closure(out, spawn_slice)), tok);
  return out;
}

/* = A tree walk (..) = */

static int iterate_walk(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_var *root = jd_get_idx(context, 0);
  jd_var *stack = jd_get_idx(context, 1);

  jd_set_void(result);
  if (jd_count(stack) == 0) return 1;

  jd_pop(stack, 1, result);
  jd_flatten(result, result);
  jd_var *slot = jd_traverse_path(root, result, 0);

  if (slot) {
    if (slot->type == ARRAY) {
      size_t sz = jd_count(slot);
      unsigned i;
      for (i = 0; i < sz; i++) {
        jd_var *itm = jd_push(jd_set_array(jd_push(stack, 1), 2), 2);
        jd_assign(itm++, result);
        jd_set_int(itm++, sz - i - 1);
      }
    }
    else if (slot->type == HASH) {
      jd_var keys = JD_INIT;
      jd_keys(&keys, slot);
      jd_sort(&keys);
      size_t sz = jd_count(&keys);
      unsigned i;
      for (i = 0; i < sz; i++) {
        jd_var *itm = jd_push(jd_set_array(jd_push(stack, 1), 2), 2);
        jd_assign(itm++, result);
        jd_assign(itm++, jd_get_idx(&keys, sz - 1 - i));
      }
      jd_release(&keys);
    }
  }

  return 1;
}

static int spawn_walk(jd_var *result, jd_var *context, jd_var *args) {
  (void) context;
  jd_int vivify = jd_get_int(result);

  jd_var *slot = jd_push(jd_set_array(
                           jd_context(jd_set_closure(result, iterate_walk)), 3), 3);
  jd_assign(slot++, args);
  jd_set_array(jd_push(jd_set_array(slot++, 50), 1), 1);
  jd_set_int(slot++, vivify);
  return 1;
}

static jd_var *make_walk_factory(jd_var *out) {
  jd_set_closure(out, spawn_walk);
  return out;
}

/* == Plumbing == */

/* = Append multiple iterators = */

static int iterate_append(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  jd_var *slot = jd_get_idx(context, 0);
  jd_var *iters = slot++;
  jd_var *idx = slot++;
  for (;;) {
    if (jd_get_int(idx) == (jd_int) jd_count(iters)) {
      jd_set_void(result);
      return 1;
    }
    jd_eval(jd_get_idx(iters, jd_get_int(idx)), result, NULL);
    if (result->type != VOID) return 1;
    jd_set_int(idx, jd_get_int(idx) + 1);
  }
}

jd_var *make_append(jd_var *out, jd_var *iters) {
  jd_var *slot = jd_push(jd_set_array(jd_context(jd_set_closure(out, iterate_append)), 2), 2);
  jd_assign(slot++, iters);
  jd_set_int(slot++, 0);
  return out;
}

static int spawn_append(jd_var *result, jd_var *context, jd_var *args) {
  (void) args;
  size_t cnt = jd_count(context);
  jd_var iters = JD_INIT;

  jd_set_array(&iters, cnt);
  jd_var *slot = jd_push(&iters, cnt);

  for (unsigned i = 0; i < cnt; i++)
    jd_eval(jd_get_idx(context, i), slot++, args);

  make_append(result, &iters);
  jd_release(&iters);
  return 1;
}

jd_var *make_append_factory(jd_var *out, jd_var *factories) {
  if (factories->type == CLOSURE)
    jd_assign(out, factories);
  else if (jd_count(factories) == 1)
    jd_assign(out, jd_get_idx(factories, 0));
  else
    jd_assign(jd_context(jd_set_closure(out, spawn_append)), factories);
  return out;
}

/* === Parser === */

void jd__path_init_parser(jd__path_parser *p, jd_var *path) {
  size_t len;
  p->path = path;
  p->sp = p->cp = jd_bytes(path, &len);
  p->ep = p->sp + len - 1;
}

static const char *single = "$@.*|,?[]()";

#define issymhead(c) ((c) == '_' || isalpha(c))
#define issymbody(c) ((c) == '_' || isalnum(c))

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
    jd_var *slice = jd_set_array_with(jd_nv(), jd_niv(JP_SLICE), NULL);
    for (;;) {
      const char *sp = (p->cp)++;
      if (!isdigit(*sp)) jd_throw("Expected a number in a slice");
      while (p->cp != p->ep && isdigit(*(p->cp)))
        (p->cp)++;
      jd_var *idx = jd_substr(jd_push(slice, 1), p->path, sp - p->sp, p->cp - sp);
      jd_numify(idx, idx);
      if (p->cp == p->ep || *(p->cp) != ':') return slice;
      (p->cp)++;
    }
  }

  if (issymhead(*(p->cp))) {
    JD_VAR(key);
    const char *sp = (p->cp)++;
    while (p->cp != p->ep && issymbody(*(p->cp)))
      (p->cp)++;
    return jd_set_array_with(jd_nv(), jd_niv(JP_KEY),
                             jd_substr(key, p->path, sp - p->sp, p->cp - sp),
                             NULL);
  }

  jd_throw("Syntax error in path: %V", p->path);
  return NULL;
}

#define PARSER_COMMON(alt, tok) \
  default:                                                     \
  jd_throw("Unhandled token: %J", tok);                        \
  case '*':                                                    \
  make_wild_factory(jd_push(alt, 1));                          \
  break;                                                       \
  case JP_DOTDOT:                                              \
  make_walk_factory(jd_push(alt, 1));                          \
  break;                                                       \
  case JP_KEY:                                                 \
  make_literal_factory(jd_push(alt, 1), jd_get_idx(tok, 1));   \
  break;                                                       \
  case JP_SLICE:                                               \
  make_slice_factory(jd_push(alt, 1), tok);                    \
  break;

static void parse_index(jd_var *alt, jd__path_parser *p) {
  JD_VAR(tok);

  for (;;) {
    tok = jd__path_token(p);
    if (!tok) jd_throw("Missing ] in path");
    switch (jd_get_int(jd_get_idx(tok, 0))) {
      PARSER_COMMON(alt, tok)
    case ']':
      return;
    case ',':
      break;
    }
  }
}

enum {
  S_INIT,
  S_REGULAR
};

/* Returns an array of closures. During iteration each closure will be called
 * with a pointer to part of a structure that is being iterated and will return
 * another closure: an iterator for all the keys, indexes or, in the case of '..',
 * sub-paths that match at the current location.
 */
static jd_var *path_parse(jd_var *out, jd__path_parser *p) {
  unsigned state = S_INIT;
  JD_2VARS(tok, alt);

  jd_set_array(out, 2);
  jd_var *path = jd_set_array(jd_push(out, 2), 10);
  jd_var *capture = jd_set_array(path + 1, 10);
  (void) capture;

  while (tok = jd__path_token(p), tok) {
    jd_int tokv = jd_get_int(jd_get_idx(tok, 0));
    jd_set_array(alt, 10);
    switch (state) {
    case S_INIT:
      switch (tokv) {
      case '$':
        make_literal_factory(jd_push(alt, 1), jd_nsv("$"));
        state = S_REGULAR;
        break;
      default:
        jd_throw("Unhandled token: %J", tok);
      }
      break;
    case S_REGULAR:
      switch (tokv) {
        PARSER_COMMON(alt, tok)
      case '.':
        break;
      case '[':
        parse_index(alt, p);
        break;
      }
      break;
    }
    if (jd_count(alt) != 0)
      make_append_factory(jd_push(path, 1), alt);
  }
  if (state == S_INIT)
    jd_throw("Bad path");
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

/* === Runtime === */

static int is_positive_int(jd_var *v) {
  jd_string *jds;
  size_t sl;
  unsigned i;

  if (JD_IS_COMPLEX(v->type)) return 0;
  if (v->type != STRING) return 1;
  jds = jd__as_string(v);
  sl = jd__string_length(jds);
  if (sl == 0) return 0;
  if (jds->data[0] == '0' && sl != 1) return 0;
  for (i = 0; i < sl; i++)
    if (!isdigit(jds->data[i])) return 0;
  return 1;
}

jd_var *jd_traverse_path(jd_var *v, jd_var *path, int vivify) {
  if (path->type == ARRAY) {
    size_t cnt = jd_count(path);
    for (unsigned i = 0; i < cnt && v; i++)
      v = jd_traverse_path(v, jd_get_idx(path, i), vivify);
    return v;
  }

  if (vivify && v->type == VOID) {
    if (is_positive_int(path)) jd_set_array(v, 1);
    else jd_set_hash(v, 1);
  }

  if (v->type != ARRAY && v->type != HASH)
    return NULL;

  return jd_get(v, path, vivify);
}

jd_var *jd__path_to_string(jd_var *volatile out, jd_var *path) {
  if (path->type == STRING)
    return jd_assign(out, path);
  scope {
    jd_join(out, jd_nsv("."), jd_flatten(jd_nv(), path));
  }
  return out;
}

static int iter_func(jd_var *result, jd_var *context, jd_var *args) {
  scope {
    jd_var *nv = jd_nv();
    jd_var *ctx = jd_get_idx(context, 0); /* array */
    jd_var *path = jd_get_idx(ctx++, 0);
    jd_var *capture = path + 1;
    jd_var *iter_stk = ctx++;
    jd_var *path_stk = ctx++;
    jd_var *var = ctx++;
    jd_int vivify = jd_get_int(ctx++);

    (void) capture;

    unsigned ipos;
    jd_var *slot_stk[ jd_count(path) + 1 ];

    jd_set_void(result);
    slot_stk[0] = var;

    for (ipos = 0; ipos < jd_count(path_stk); ipos++)
      slot_stk[ipos + 1] = jd_traverse_path(
        slot_stk[ipos], jd_get_idx(path_stk, ipos), vivify);

    while (jd_count(path_stk) < jd_count(path)) {
      if (jd_count(iter_stk) < jd_count(path_stk)) jd_die("Oops!");
      if (ipos = jd_count(iter_stk), ipos == jd_count(path_stk)) {
        jd_var *slot = jd_push(iter_stk, 1);
        /* cheeky: pass vivify flag in via result */
        jd_set_int(slot, vivify);
        jd_eval(jd_get_idx(path, ipos), slot, slot_stk[ipos]);
      }

      jd_eval(jd_get_idx(iter_stk, -1), nv, NULL);

      if (nv->type != VOID) {
        ipos = jd_count(path_stk);
        slot_stk[ipos + 1] = jd_traverse_path(slot_stk[ipos], nv, vivify);
        if (slot_stk[ipos + 1]) {
          jd_assign(jd_push(path_stk, 1), nv);
          continue;
        }
      }

      if (jd_count(path_stk) == 0) JD_RETURN(1);
      jd_pop(path_stk, 1, NULL);
      jd_pop(iter_stk, 1, NULL);
    }

    jd_set_object(result, slot_stk[jd_count(path_stk)], NULL);

    if (args) {
      /* return [ path, captures ] */
      jd_set_array(args, 2);
      jd_var *rs = jd_push(args, 2);
      jd__path_to_string(rs++, path_stk); /* path */
      jd_set_array(rs++, 1); /* TODO captures */
    }

    jd_pop(path_stk, 1, NULL);
  }

  return 1;
}

jd_var *jd_path_iter(jd_var *iter, jd_var *v, jd_var *path, int vivify) {
  scope {
    jd_var *ctx = jd_set_array(jd_context(jd_set_closure(iter, iter_func)), 10);
    /* build context */
    jd_var *slot = jd_push(ctx, 5);
    jd_assign(slot++, jd__path_compile(jd__path_to_string(jd_nv(), path)));
    jd_set_array(slot++, 10); /* iter_stk */
    jd_set_array(slot++, 10); /* path_stk */
    /* push { "$": v } */
    jd_assign(jd_get_ks(jd_set_hash(slot++, 1), "$", 1), v); /* var */
    jd_set_bool(slot++, vivify);
  }

  return iter;
}

jd_var *jd_path_next(jd_var *iter, jd_var *path, jd_var *captures) {
  jd_var next = JD_INIT;
  jd_var args = JD_INIT;

  if (path || captures) {
    if (path) jd_set_void(path);
    if (captures) jd_set_void(captures);
    jd_eval(iter, &next, &args);
  }
  else {
    jd_eval(iter, &next, NULL);
  }

  jd_var *rs = NULL;

  if (next.type != VOID) {
    rs = jd_ptr(&next);
    if (path) jd_assign(path, jd_get_idx(&args, 0));
    if (captures) jd_assign(captures, jd_get_idx(&args, 1));
  }

  jd_release(&next);
  jd_release(&args);

  return rs;
}

jd_var *jd_path_object(jd_var *iter) {
  return jd_get_ks(jd_get_idx(jd_context(iter), 3), "$", 0);
}

jd_var *jd_get_context(jd_var *root, jd_var *path, int vivify) {
  jd_var *rv = NULL;
  scope {
    jd_var *iter = jd_nv();
    jd_path_iter(iter, root, path, vivify);
    rv = jd_path_next(iter, NULL, NULL);
    jd_var *po = jd_path_object(iter);
    if (rv == po) rv = root;
    if (vivify) jd_assign(root, po);
  }
  return rv;
}

static jd_var *getter(jd_var *root, const char *path, va_list ap, int vivify) {
  jd_var *rv = NULL;
  scope {
    JD_VAR(pv);
    jd_vsprintf(pv, path, ap);
    rv = jd_get_context(root, pv, vivify);
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
