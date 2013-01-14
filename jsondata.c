/* jsondata.c */

#include "jsondata.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  void *m = malloc(sz);
  if (!m) oom();
  memset(m, 0, sz);
  return m;
}

void jd_free(void *m) {
  free(m);
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

jd_var *jd_set_string(jd_var *v, const char *s) {
  jd_string *jds = jd_string_from(s);
  jd_release(v);
  v->type = STRING;
  v->v.s = jds;
  return v;
}

jd_var *jd_set_array(jd_var *v, size_t size) {
  jd_array *jda = jd_array_new(size);
  jd_release(v);
  v->type = ARRAY;
  v->v.a = jda;
  return v;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
