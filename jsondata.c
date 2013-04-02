/* jsondata.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jd_private.h"
#include "jsondata.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X(x) #x,
static const char *typename[] = {
  JD_TYPE_LIST
  NULL
};
#undef X

void *(*jd_alloc_hook)(size_t) = malloc;
void (*jd_free_hook)(void *) = free;

void jd_die(const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  fprintf(stderr, "Fatal: ");
  vfprintf(stderr, msg, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  abort();
}

static void oom(void) {
  jd_die("Out of memory");
}

static void notnull(void *v) {
  if (!v) jd_throw("Null pointer");
}

void *jd_alloc(size_t sz) {
  void *m = jd_alloc_hook(sz);
  if (!m) oom();
  memset(m, 0, sz);
  return m;
}

void jd_free(void *m) {
  jd_free_hook(m);
}

void jd_release(jd_var *v) {
  notnull(v);
  switch (v->type) {
  case VOID:
  case BOOL:
  case INTEGER:
  case REAL:
    break;
  case STRING:
    jd__string_release(v->v.s);
    break;
  case ARRAY:
    jd__array_release(v->v.a);
    break;
  case HASH:
    jd__hash_release(v->v.h);
    break;
  case CLOSURE:
    jd__closure_release(v->v.c);
    break;
  case OBJECT:
    jd__object_release(v->v.o);
    break;
  default:
    jd_throw("Unhandled type in release: %V", v);
    break;
  }
  memset(v, 0, sizeof(*v));
}

void jd_retain(jd_var *v) {
  notnull(v);
  switch (v->type) {
  case VOID:
  case BOOL:
  case INTEGER:
  case REAL:
    break;
  case STRING:
    jd__string_retain(v->v.s);
    break;
  case ARRAY:
    jd__array_retain(v->v.a);
    break;
  case HASH:
    jd__hash_retain(v->v.h);
    break;
  case CLOSURE:
    jd__closure_retain(v->v.c);
    break;
  case OBJECT:
    jd__object_retain(v->v.o);
    break;
  default:
    jd_throw("Unhandled type in retain: %V", v);
    break;
  }
}

jd_var *jd_assign(jd_var *dst, jd_var *src) {
  if (dst == src) return dst;
  jd_retain(src);
  jd_release(dst);
  *dst = *src;
  return dst;
}

static jd_var *set_string(jd_var *v, jd_string *jds) {
  jd_release(v);
  v->type = STRING;
  v->v.s = jds;
  return v;
}

jd_var *jd_set_empty_string(jd_var *v, size_t size) {
  return set_string(v, jd__string_new(size));
}

jd_var *jd_set_string(jd_var *v, const char *s) {
  return set_string(v, jd__string_from(s));
}

jd_var *jd_set_bytes(jd_var *v, const void *s, size_t size) {
  return set_string(v, jd__string_from_bytes(s, size));
}

jd_var *jd_set_array(jd_var *v, size_t size) {
  jd_release(v);
  v->type = ARRAY;
  v->v.a = jd__array_new(size);
  return v;
}

static size_t varg_count(va_list ap) {
  va_list aq;
  size_t count;
  va_copy(aq, ap);
  for (count = 0; va_arg(aq, jd_var *); count++) ;
  va_end(aq);
  return count;
}

jd_var *jd_set_array_with(jd_var *v, ...) {
  va_list ap;
  va_start(ap, v);
  size_t count = varg_count(ap);

  jd_set_array(v, count);

  jd_var *vv, *slot = jd_push(v, count);
  while (vv = va_arg(ap, jd_var *), vv)
    jd_assign(slot++, vv);

  va_end(ap);

  return v;
}

jd_var *jd_set_hash_with(jd_var *v, ...) {
  va_list ap;
  va_start(ap, v);
  size_t count = varg_count(ap);

  if (count & 1)
    jd_throw("Odd number of elements in hash initializer: %lu",
             (unsigned long) count);

  jd_set_hash(v, count / 2);

  jd_var *kv;
  while (kv = va_arg(ap, jd_var *), kv)
    jd_assign(jd_get_key(v, kv, 1), va_arg(ap, jd_var *));

  va_end(ap);

  return v;
}

jd_var *jd_set_hash(jd_var *v, size_t size) {
  jd_release(v);
  v->type = HASH;
  v->v.h = jd__hash_new(size);
  return v;
}

jd_var *jd_set_closure(jd_var *v, jd_closure_func f) {
  jd_release(v);
  v->type = CLOSURE;
  v->v.c = jd__closure_new(f);
  return v;
}

jd_var *jd_set_object(jd_var *v, void *o, void (*free)(void *)) {
  jd_release(v);
  v->type = OBJECT;
  v->v.o = jd__object_new(o, free);
  return v;
}

jd_var *jd_set_int(jd_var *v, jd_int i) {
  jd_release(v);
  v->type = INTEGER;
  v->v.i = i;
  return v;
}

jd_var *jd_set_real(jd_var *v, double r) {
  jd_release(v);
  v->type = REAL;
  v->v.r = r;
  return v;
}

jd_var *jd_set_bool(jd_var *v, int b) {
  jd_release(v);
  v->type = BOOL;
  v->v.b = !!b;
  return v;
}

jd_var *jd_set_void(jd_var *v) {
  jd_release(v);
  v->type = VOID;
  return v;
}


jd_string *jd__as_string(jd_var *v) {
  if (v->type != STRING) jd_throw("Not a string");
  return v->v.s;
}

jd_array *jd__as_array(jd_var *v) {
  if (v->type != ARRAY) jd_throw("Not an array");
  return v->v.a;
}

jd_hash *jd__as_hash(jd_var *v) {
  if (v->type != HASH) jd_throw("Not a hash");
  return v->v.h;
}

jd_closure *jd__as_closure(jd_var *v) {
  if (v->type != CLOSURE) jd_throw("Not a closure");
  return v->v.c;
}

jd_object *jd__as_object(jd_var *v) {
  if (v->type != OBJECT) jd_throw("Not an object");
  return v->v.o;
}

size_t jd_length(jd_var *v) {
  return jd__string_length(jd__as_string(v));
}

jd_var *jd_append(jd_var *v, jd_var *v2) {
  switch (v->type) {
  case STRING:
    jd__string_append(jd__as_string(v), v2);
    break;
  case ARRAY:
    jd__array_append(jd__as_array(v), v2);
    break;
  default:
    jd_throw("Can't append"); /* TODO type name */
  }
  return v;
}

jd_var *jd_append_bytes(jd_var *v, const void *b, size_t size) {
  jd__string_append_bytes(jd__as_string(v), b, size);
  return v;
}

size_t jd_count(jd_var *v) {
  switch (v->type) {
  case ARRAY:
    return jd__array_count(jd__as_array(v));
  case HASH:
    return jd__hash_count(jd__as_hash(v));
  default:
    return 0;
  }
}

jd_var *jd_insert(jd_var *v, int idx, size_t count) {
  return jd__array_insert(jd__as_array(v), idx, count);
}

size_t jd_remove(jd_var *v, int idx, size_t count, jd_var *slot) {
  return jd__array_remove(jd__as_array(v), idx, count, slot);
}

jd_var *jd_push(jd_var *v, size_t count) {
  return jd__array_push(jd__as_array(v), count);
}

size_t jd_pop(jd_var *v, size_t count, jd_var *slot) {
  return jd__array_pop(jd__as_array(v), count, slot);
}

jd_var *jd_unshift(jd_var *v, size_t count) {
  return jd__array_unshift(jd__as_array(v), count);
}

size_t jd_shift(jd_var *v, size_t count, jd_var *slot) {
  return jd__array_shift(jd__as_array(v), count, slot);
}

jd_var *jd_get_idx(jd_var *v, int idx) {
  return jd__array_get(jd__as_array(v), idx);
}

jd_var *jd_join(jd_var *out, jd_var *sep, jd_var *ar) {
  return jd__array_join(out, sep, jd__as_array(ar));
}

jd_var *jd_get_key(jd_var *v, jd_var *key, int vivify) {
  return jd__hash_get(jd__as_hash(v), key, vivify);
}

jd_var *jd_get_ks(jd_var *v, const char *key, int vivify) {
  jd_var kv = JD_INIT;
  jd_set_string(&kv, key);
  jd_var *rv = jd_get_key(v, &kv, vivify);
  jd_release(&kv);
  return rv;
}

jd_var *jd_get(jd_var *v, jd_var *key, int vivify) {
  switch (v->type) {
  case HASH:
    return jd_get_key(v, key, vivify);
  case ARRAY:
    if (vivify) {
      jd_int idx = jd_get_int(key);
      jd_int diff = idx - (jd_int) jd_count(v);
      return diff >= 0 ? jd_push(v, diff + 1) + diff : jd_get_idx(v, idx);
    }
    else {
      jd_int idx = jd_get_int(key);
      return idx < (jd_int) jd_count(v) ? jd_get_idx(v, idx) : NULL;
    }
  default:
    jd_throw("Bad type for get"); /* TODO typename in error messages */
  }
}

int jd_delete_key(jd_var *v, jd_var *key, jd_var *slot) {
  return jd__hash_delete(jd__as_hash(v), key, slot);
}

int jd_delete_ks(jd_var *v, const char *key, jd_var *slot) {
  jd_var kv = JD_INIT;
  jd_set_string(&kv, key);
  int rv = jd_delete_key(v, &kv, slot);
  jd_release(&kv);
  return rv;
}

static int compare(jd_var *a, jd_var *b) {

  /* a->type <= b->type so we can compare integers with
   * other integers or reals here.
   */

  if (a->type == INTEGER) {
    switch (b->type) {
    case INTEGER:
      return a->v.i < b->v.i ? -1 : a->v.i > b->v.i ? 1 : 0;
    case REAL:
      return a->v.i < b->v.r ? -1 : a->v.i > b->v.r ? 1 : 0;
    default:
      break;
    }
  }

  /* If types differ default to type based comparison */

  if (b->type != a->type) return a->type - b->type;

  /* Once we get here a->type == b->type */

  switch (a->type) {
  case BOOL:
    return a->v.b < b->v.b ? -1 : a->v.b > b->v.b ? 1 : 0;
  case REAL:
    return a->v.r < b->v.r ? -1 : a->v.r > b->v.r ? 1 : 0;
  case STRING:
    return jd__string_compare(jd__as_string(a), b);
  default:
    jd_throw("Can't compare");
    return 0;
  }
}

int jd_compare(jd_var *a, jd_var *b) {
  if (a == NULL || b == NULL)
    return a == NULL ? b == NULL ? 0 : -1 : 1;
  if (a->type > b->type) return -compare(b, a);
  return compare(a, b);
}

unsigned long jd_hashcalc(jd_var *v) {
  /* Flatten INTEGER and REAL so that 0 == 0.0 */
  jd_type t = v->type == REAL ? INTEGER : v->type;
  int rc = 0;
  switch (v->type) {
  case STRING:
    rc = jd__string_hashcalc(jd__as_string(v), t);
    break;
  default: {
    jd_var vv = JD_INIT;
    jd_stringify(&vv, v);
    rc = jd__string_hashcalc(jd__as_string(&vv), t);
    jd_release(&vv);
  }
  break;
  }
  return rc;
}

jd_var *jd_keys(jd_var *keys, jd_var *v) {
  return jd__hash_keys(jd__as_hash(v), keys);
}

jd_var *jd_stringify(jd_var *out, jd_var *v) {
  jd_var tmp = JD_INIT;
  if (v) {
    switch (v->type) {
    case VOID:
      jd_set_string(&tmp, "null");
      break;
    case BOOL:
      jd_set_string(&tmp, v->v.b ? "true" : "false");
      break;
    case INTEGER:
      jd_printf(&tmp, JD_INT_FMT, v->v.i);
      break;
    case REAL:
      jd_printf(&tmp, "%g", v->v.r);
      break;
    case STRING:
      jd_assign(&tmp, v);
      break;
    case HASH:
    case ARRAY:
      jd_to_json(&tmp, v);
      break;
    default:
      if (v->type < MAXTYPE)
        jd_printf(&tmp, "<%s:%p>", typename[v->type], v);
      else
        jd_printf(&tmp, "<UNKNOWN(%u):%p>", v->type, v);
      break;
    }
  }
  else {
    jd_set_string(&tmp, "<NULL>");
  }
  jd_assign(out, &tmp);
  jd_release(&tmp);
  return out;
}

jd_var *jd_numify(jd_var *out, jd_var *v) {
  switch (v->type) {
  case VOID:
  case BOOL:
  case INTEGER:
  case REAL:
    return jd_assign(out, v);
  case STRING:
    return jd__string_numify(jd__as_string(v), out);
  case ARRAY:
  case HASH:
    return jd_set_int(out, jd_count(v));
  default:
    jd_throw("Can't numify");
    return NULL;
  }
}

int jd_test(jd_var *v) {
  switch (v->type) {
  case STRING:
    return jd_length(v) ? 1 : 0;
  case REAL:
    return jd_get_real(v) ? 1 : 0;
  default:
    return jd_get_int(v) ? 1 : 0;
  }
}

jd_var *jd_substr(jd_var *out, jd_var *v, int from, int len) {
  jd__string_sub(jd__as_string(v), from, len, out);
  return out;
}

int jd_find(jd_var *haystack, jd_var *needle, int pos) {
  return jd__string_find(jd__as_string(haystack), needle, pos);
}

jd_var *jd_split(jd_var *out, jd_var *v, jd_var *sep) {
  return jd__string_split(jd__as_string(v), sep, out);
}

#define CAST(vtype, name)                    \
  vtype name(jd_var *v) {                    \
    vtype rv = 0;                            \
    jd_var tmp = JD_INIT;                    \
    jd_numify(&tmp, v);                      \
    switch (tmp.type) {                      \
    case INTEGER:                            \
      rv = (vtype) tmp.v.i;                  \
      break;                                 \
    case REAL:                               \
      rv = (vtype) tmp.v.r;                  \
      break;                                 \
    case BOOL:                               \
      rv = (vtype) tmp.v.b;                  \
      break;                                 \
    default:                                 \
      jd_throw("Oops - expected a numeric"); \
    case VOID:                               \
      rv = 0;                                \
      break;                                 \
    }                                        \
    jd_release(&tmp);                        \
    return rv;                               \
  }

CAST(jd_int, jd_get_int)
CAST(double, jd_get_real)

jd_var *jd_sortv(jd_var *v, int (*cmp)(jd_var *, jd_var *)) {
  jd__array_sort(jd__as_array(v), cmp);
  return v;
}

jd_var *jd_sort(jd_var *v) {
  return jd_sortv(v, jd_compare);
}

const char *jd_bytes(jd_var *v, size_t *sp) {
  return jd__string_bytes(jd__as_string(v), sp);
}

jd_var *jd_merge(jd_var *out, jd_var *v, int deep) {
  /* TODO this isn't really a deep hash merge */
  jd__hash_merge(out, jd__as_hash(v), deep);
  return out;
}

jd_var *jd_clone(jd_var *out, jd_var *v, int deep) {
  switch (v->type) {
  case STRING:
    return jd_substr(out, v, 0, jd_length(v));
  case ARRAY:
    return jd__array_clone(out, jd__as_array(v), deep);
  case HASH:
    return jd__hash_clone(out, jd__as_hash(v), deep);
  case CLOSURE:
    return jd__closure_clone(out, jd__as_closure(v), deep);
  default:
    return jd_assign(out, v);
  }
}

static jd_var *subref(jd_var *out, jd_var *v, int from, int len) {
  if (from == 0 && len == (int) jd_length(v)) return jd_assign(out, v);
  return jd_substr(out, v, from, len);
}

jd_var *jd_ltrim(jd_var *out, jd_var *v) {
  size_t size;
  const char *buf = jd_bytes(v, &size);
  unsigned i;
  for (i = 0; i < size && isspace(buf[i]); i++) ;
  return subref(out, v, i, size - 1 - i);
}

jd_var *jd_rtrim(jd_var *out, jd_var *v) {
  size_t size;
  const char *buf = jd_bytes(v, &size);
  unsigned i;
  for (i = size - 1; i > 0 && isspace(buf[i - 1]); i--) ;
  return subref(out, v, 0, i);
}

jd_var *jd_trim(jd_var *out, jd_var *v) {
  return jd_ltrim(out, jd_rtrim(out, v));
}

jd_var *jd_context(jd_var *v) {
  return jd__closure_context(jd__as_closure(v));
}

jd_var *jd_eval(jd_var *cl, jd_var *rv, jd_var *arg) {
  jd__closure_call(jd__as_closure(cl), rv, arg);
  return rv;
}

void jd_call(jd_var *cl, jd_var *arg) {
  JD_SCOPE {
    JD_VAR(rv);
    jd_eval(cl, rv, arg);
  }
}

jd_var *jd_promise(jd_var *out, jd_var *v, jd_var *args) {
  if (v->type == CLOSURE)
    jd_eval(v, out, args);
  else
    jd_assign(out, v);
  return out;
}

void *jd_ptr(jd_var *v) {
  return jd__as_object(v)->o;
}

jd_var *jd_version(jd_var *out) {
  jd_set_hash(out, 3);
  jd_set_string(jd_get_ks(out, "version", 1), VERSION);
  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
