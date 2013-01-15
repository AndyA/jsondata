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
  switch (v->type) {
  case VOID:
  case BOOL:
  case INTEGER:
  case REAL:
    break;
  case STRING:
    jd_string_release(v->v.s);
    break;
  case ARRAY:
    jd_array_release(v->v.a);
    break;
  case HASH:
    break;
  }
  v->type = VOID;
}

void jd_retain(jd_var *v) {
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
  jd_array *jda = jd_array_new(size);
  jd_release(v);
  v->type = ARRAY;
  v->v.a = jda;
  return v;
}

jd_string *jd_as_string(jd_var *v) {
  if (v->type != STRING) jd_die("Not an string");
  return v->v.s;
}

jd_array *jd_as_array(jd_var *v) {
  if (v->type != ARRAY) jd_die("Not an array");
  return v->v.a;
}

size_t jd_length(jd_var *v) {
  return jd_string_length(jd_as_string(v));
}

jd_var *jd_append(jd_var *v, jd_var *v2) {
  jd_string_append(jd_as_string(v), v2);
  return v;
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

jd_var *jd_get(jd_var *v, int idx) {
  return jd_array_get(jd_as_array(v), idx);
}

jd_var *jd_join(jd_var *out, jd_var *sep, jd_var *ar) {
  return jd_array_join(out, sep, jd_as_array(ar));
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
