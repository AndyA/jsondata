/* jd_string.c */

#include "jsondata.h"

#include <string.h>

jd_string *jd_string_init(jd_string *jds, size_t size) {
  jds->data = jd_alloc(size);
  jds->size = size;
  jds->used = 0;
  jds->hdr.refs = 1; /* or maybe 0... */
  return jds;
}

jd_string *jd_string_new(size_t size) {
  jd_string *jds = jd_alloc(sizeof(jd_string));
  jd_string_init(jds, size);
  jds->used = 1; /* trailing \0 */
  return jds;
}

jd_string *jd_string_from(const char *s) {
  size_t len = strlen(s) + 1;
  jd_string *jds = jd_string_new(len);
  memcpy(jds->data, s, len);
  jds->used = len;
  return jds;
}

jd_string *jd_string_ensure(jd_string *jds, size_t size) {
  if (jds->size < size) {
    char *nstr = jd_alloc(size);
    memcpy(nstr, jds->data, jds->used);
    jd_free(jds->data);
    jds->data = nstr;
  }
  return jds;
}

jd_string *jd_string_space(jd_string *jds, size_t minspace) {
  if (jds->size - jds->used < minspace) {
    size_t newsize = jds->used + minspace;
    if (newsize < jds->size * 2) newsize = jds->size * 2;
    return jd_string_ensure(jds, newsize);
  }
  return jds;
}

void jd_string_free(jd_string *jds) {
  jd_free(jds->data);
  jd_free(jds);
}

jd_string *jd_string_retain(jd_string *jds) {
  jds->hdr.refs++;
  return jds;
}

jd_string *jd_string_release(jd_string *jds) {
  if (jds->hdr.refs <= 1) {
    jd_string_free(jds);
    return NULL;
  }
  jds->hdr.refs--;
  return jds;
}

size_t jd_string_length(jd_string *jds) {
  return jds->used - 1;
}

jd_string *jd_string_append(jd_string *jds, jd_var *v) {
  jd_string *vs = jd_as_string(v);
  size_t len = jd_string_length(vs);
  jd_string_space(jds, len);
  memcpy(jds->data + jds->used - 1, vs->data, len + 1);
  jds->used += len;
  return jds;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
