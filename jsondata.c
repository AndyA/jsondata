/* jsondata.c */

#include "jsondata.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *(*jd_alloc_hook)(size_t) = malloc;
void (*jd_free_hook)(void *) = free;

void jd_die(const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  fprintf(stderr, "Fatal: ");
  vfprintf(stderr, msg, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

static void oom(void) {
  jd_die("Out of memory");
}

static void notnull(void *v) {
  if (!v) jd_die("Null pointer");
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
  void *rc = NULL;
  notnull(v);
  switch (v->type) {
  case VOID:
  case BOOL:
  case INTEGER:
  case REAL:
    break;
  case STRING:
    rc = jd_string_release(v->v.s);
    break;
  case ARRAY:
    rc = jd_array_release(v->v.a);
    break;
  case HASH:
    rc = jd_hash_release(v->v.h);
    break;
  }
  if (!rc) memset(v, 0, sizeof(*v));
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
    jd_string_retain(v->v.s);
    break;
  case ARRAY:
    jd_array_retain(v->v.a);
    break;
  case HASH:
    jd_hash_retain(v->v.h);
    break;
  }
}

jd_var *jd_assign(jd_var *dst, jd_var *src) {
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
  return set_string(v, jd_string_new(size));
}

jd_var *jd_set_string(jd_var *v, const char *s) {
  return set_string(v, jd_string_from(s));
}

jd_var *jd_set_array(jd_var *v, size_t size) {
  jd_release(v);
  v->type = ARRAY;
  v->v.a = jd_array_new(size);
  return v;
}

jd_var *jd_set_hash(jd_var *v, size_t size) {
  jd_release(v);
  v->type = HASH;
  v->v.h = jd_hash_new(size);
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


jd_string *jd_as_string(jd_var *v) {
  if (v->type != STRING) jd_die("Not a string");
  return v->v.s;
}

jd_array *jd_as_array(jd_var *v) {
  if (v->type != ARRAY) jd_die("Not an array");
  return v->v.a;
}

jd_hash *jd_as_hash(jd_var *v) {
  if (v->type != HASH) jd_die("Not a hash");
  return v->v.h;
}

size_t jd_length(jd_var *v) {
  return jd_string_length(jd_as_string(v));
}

jd_var *jd_append(jd_var *v, jd_var *v2) {
  switch (v->type) {
  case STRING:
    jd_string_append(jd_as_string(v), v2);
    break;
  case ARRAY:
    jd_array_append(jd_as_array(v), v2);
    break;
  default:
    jd_die("Can't append"); /* TODO type name */
  }
  return v;
}

jd_var *jd_append_bytes(jd_var *v, const void *b, size_t size) {
  jd_string_append_bytes(jd_as_string(v), b, size);
  return v;
}

size_t jd_count(jd_var *v) {
  switch (v->type) {
  case ARRAY:
    return jd_array_count(jd_as_array(v));
  case HASH:
    return jd_hash_count(jd_as_hash(v));
  default:
    return 0;
  }
}

jd_var *jd_insert(jd_var *v, int idx, size_t count) {
  return jd_array_insert(jd_as_array(v), idx, count);
}

size_t jd_remove(jd_var *v, int idx, size_t count, jd_var *slot) {
  return jd_array_remove(jd_as_array(v), idx, count, slot);
}

jd_var *jd_push(jd_var *v, size_t count) {
  return jd_array_push(jd_as_array(v), count);
}

size_t jd_pop(jd_var *v, size_t count, jd_var *slot) {
  return jd_array_pop(jd_as_array(v), count, slot);
}

jd_var *jd_unshift(jd_var *v, size_t count) {
  return jd_array_unshift(jd_as_array(v), count);
}

size_t jd_shift(jd_var *v, size_t count, jd_var *slot) {
  return jd_array_shift(jd_as_array(v), count, slot);
}

jd_var *jd_get_idx(jd_var *v, int idx) {
  return jd_array_get(jd_as_array(v), idx);
}

jd_var *jd_join(jd_var *out, jd_var *sep, jd_var *ar) {
  return jd_array_join(out, sep, jd_as_array(ar));
}

jd_var *jd_get_key(jd_var *v, jd_var *key, int vivify) {
  return jd_hash_get(jd_as_hash(v), key, vivify);
}

int jd_delete_key(jd_var *v, jd_var *key, jd_var *slot) {
  return jd_hash_delete(jd_as_hash(v), key, slot);
}

int jd_compare(jd_var *a, jd_var *b) {
  if (b->type != a->type) return a->type - b->type;
  switch (a->type) {
  case STRING:
    return jd_string_compare(jd_as_string(a), b);
  default:
    jd_die("Can't compare");
    return 0;
  }
}

unsigned long jd_hashcalc(jd_var *v) {
  switch (v->type) {
  case STRING:
    return jd_string_hashcalc(jd_as_string(v));
  default:
    jd_die("Can't compute hash");
    return 0;
  }
}

jd_var *jd_keys(jd_var *v, jd_var *keys) {
  return jd_hash_keys(jd_as_hash(v), keys);
}

jd_var *jd_printf(jd_var *v, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  jd_string_vprintf(jd_as_string(v), fmt, ap);
  va_end(ap);
  return v;
}

jd_var *jd_stringify(jd_var *out, jd_var *v) {
  /* TODO move conversion of out into a string into jd_printf */
  jd_var tmp = JD_INIT;
  jd_set_empty_string(&tmp, 20);
  switch (v->type) {
  case VOID:
    jd_set_string(&tmp, "null");
    break;
  case BOOL:
    jd_set_string(&tmp, v->v.b ? "true" : "false");
    break;
  case INTEGER:
    jd_printf(&tmp, "%lld", v->v.i);
    break;
  case REAL:
    jd_printf(&tmp, "%g", v->v.r);
    break;
  case STRING:
    jd_assign(&tmp, v);
    break;
  default:
    jd_die("Can't stringify");
    return NULL;
  }
  jd_assign(out, &tmp);
  jd_release(&tmp);
  return out;
}

jd_var *jd_numify(jd_var *out, jd_var *v) {
  switch (v->type) {
  case HASH:
  case ARRAY:
    jd_die("Can't numify");
    return NULL;
  default:
    return jd_assign(out, v);
  case STRING:
    return jd_string_numify(jd_as_string(v), out);
  }
}

jd_var *jd_substr(jd_var *out, jd_var *v, int from, int len) {
  jd_string_sub(jd_as_string(v), from, len, out);
  return out;
}

int jd_find(jd_var *haystack, jd_var *needle, int pos) {
  return jd_string_find(jd_as_string(haystack), needle, pos);
}

jd_var *jd_split(jd_var *out, jd_var *v, jd_var *sep) {
  return jd_string_split(jd_as_string(v), sep, out);
}

#define CAST(vtype, name) \
  vtype name(jd_var *v) {                   \
    jd_var tmp = JD_INIT;                   \
    vtype rv;                               \
    jd_numify(&tmp, v);                     \
    switch (tmp.type) {                     \
    case INTEGER:                           \
      rv = (vtype) tmp.v.i;                 \
      break;                                \
    case REAL:                              \
      rv = (vtype) tmp.v.r;                 \
      break;                                \
    case BOOL:                              \
      rv = (vtype) tmp.v.b;                 \
      break;                                \
    default:                                \
      jd_die("Oops - expected a numeric");  \
    case VOID:                              \
      rv = 0;                               \
      break;                                \
    }                                       \
    jd_release(&tmp);                       \
    return rv;                              \
  }

CAST(jd_int, jd_get_int)
CAST(double, jd_get_real)

jd_var *jd_sort(jd_var *v) {
  jd_array_sort(jd_as_array(v));
  return v;
}

const char *jd_bytes(jd_var *v, size_t *sp) {
  return jd_string_bytes(jd_as_string(v), sp);
}

jd_var *jd_merge(jd_var *out, jd_var *v, int deep) {
  jd_hash_merge(out, jd_as_hash(v), deep);
  return out;
}

jd_var *jd_clone(jd_var *out, jd_var *v, int deep) {
  switch (v->type) {
  case VOID:
  case BOOL:
  case INTEGER:
  case REAL:
    *out = *v;
    return out;
  case STRING:
    return jd_substr(out, v, 0, jd_length(v));
  case ARRAY:
    return jd_array_clone(out, jd_as_array(v), deep);
  case HASH:
    return jd_hash_clone(out, jd_as_hash(v), deep);
  }
  return NULL;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
