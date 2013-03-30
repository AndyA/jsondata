/* jd_string.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jd_private.h"
#include "jsondata.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
  if (size == 0) size = 1;
  jd_string_init(jds, size);
  jds->used = 1; /* trailing \0 */
  return jds;
}

jd_string *jd_string_empty(jd_string *jds) {
  jds->used = 1;
  jds->data[0] = '\0';
  return jds;
}

jd_string *jd_string_from_bytes(const char *s, size_t size) {
  jd_string *jds = jd_string_new(size + 1);
  memcpy(jds->data, s, size);
  jds->data[size] = '\0';
  jds->used = size + 1;
  return jds;
}

jd_string *jd_string_from(const char *s) {
  return jd_string_from_bytes(s, strlen(s));
}

jd_string *jd_string_ensure(jd_string *jds, size_t size) {
  if (jds->size < size) {
    char *nstr = jd_alloc(size);
    memcpy(nstr, jds->data, jds->used);
    jd_free(jds->data);
    jds->data = nstr;
    jds->size = size;
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

void jd_string_retain(jd_string *jds) {
  jds->hdr.refs++;
}

void jd_string_release(jd_string *jds) {
  if (jds->hdr.refs-- <= 1)
    jd_string_free(jds);
}

size_t jd_string_length(jd_string *jds) {
  return jds->used - 1;
}

jd_string *jd_string_append(jd_string *jds, jd_var *v) {
  jd_string *vs = jd_as_string(v);
  size_t len = jd_string_length(vs);
  jd_string_space(jds, len);
  memmove(jds->data + jds->used - 1, vs->data, len + 1);
  jds->used += len;
  return jds;
}

jd_string *jd_string_append_bytes(jd_string *jds, const void *b, size_t size) {
  jd_string_space(jds, size);
  memcpy(jds->data + jds->used - 1, b, size);
  jds->used += size;
  jds->data[jds->used - 1] = '\0';
  return jds;
}

int jd_string_compare(jd_string *jds, jd_var *v) {
  jd_string *vs = jd_as_string(v);
  size_t la = jd_string_length(jds);
  size_t lb = jd_string_length(vs);
  size_t lc = la < lb ? la : lb;
  int cmp = memcmp(jds->data, vs->data, lc);
  if (cmp) return cmp;
  return la < lb ? -1 : la > lb ? 1 : 0;
}

unsigned long jd_string_hashcalc(jd_string *jds, jd_type t) {
  unsigned long h = t;
  size_t len = jd_string_length(jds);
  unsigned i;
  for (i = 0; i < len; i++) {
    h = 31 * h + jds->data[i];
  }
  return h;
}

jd_var *jd_string_sub(jd_string *jds, int from, int len, jd_var *out) {
  size_t sl = jd_string_length(jds);

  if (from < 0) from += sl;
  if (len <= 0 || from < 0 || from >= sl) {
    jd_set_string(out, "");
    return out;
  }

  JD_SCOPE {
    JD_VAR(tmp);
    jd_string *jo;
    if (from + len > sl) len = sl - from;
    jd_set_empty_string(tmp, len + 1);
    jo = jd_as_string(tmp);
    memcpy(jo->data, jds->data + from, len);
    jo->data[len] = '\0';
    jo->used = len + 1;
    jd_assign(out, tmp);
  }
  return out;
}

static const char *memfind(const char *haystack, size_t hslen,
                           const char *needle, size_t nlen) {
  const char *hsend = haystack + hslen - nlen;

  while (haystack <= hsend) {
    if (memcmp(haystack, needle, nlen) == 0) return haystack;
    haystack++;
  }

  return NULL;
}

int jd_string_find(jd_string *jds, jd_var *pat, int from) {
  jd_string *ps = jd_as_string(pat);
  size_t sl = jd_string_length(jds);
  size_t pl = jd_string_length(ps);
  if (from < 0) from += sl;
  if (from < 0 || from + pl > sl) return -1;
  const char *hit = memfind(jds->data + from, sl - from, ps->data, pl);
  if (hit) return hit - jds->data;
  return -1;
}

jd_var *jd_string_split(jd_string *jds, jd_var *pat, jd_var *out) {
  int pos;
  jd_set_array(out, 10);
  for (pos = 0; ;) {
    int hit = jd_string_find(jds, pat, pos);
    if (hit == -1) break;
    jd_string_sub(jds, pos, hit - pos, jd_push(out, 1));
    pos = hit + 1;
  }
  jd_string_sub(jds, pos, jd_string_length(jds) - pos, jd_push(out, 1));
  return out;
}

static int str_is(jd_string *jds, const char *s) {
  size_t jl = jd_string_length(jds);
  size_t sl = strlen(s);
  return jl == sl && memcmp(jds->data, s, jl) == 0;
}

jd_var *jd_string_numify(jd_string *jds, jd_var *out) {
  char *end;
  jd_int iv;
  double rv;
  size_t sl;

  if (str_is(jds, "true"))
    return jd_set_bool(out, 1);
  if (str_is(jds, "false"))
    return jd_set_bool(out, 0);
  if (str_is(jds, "null"))
    return jd_set_void(out);

  sl = jd_string_length(jds);
  iv = (jd_int) JD_STRTOINT(jds->data, &end, 10);
  if (end - jds->data == sl)
    return jd_set_int(out, iv);

  rv = strtod(jds->data, &end);
  if (end - jds->data == sl)
    return jd_set_real(out, rv);

  jd_throw("Can't convert to a number");

  return NULL;
}

const char *jd_string_bytes(jd_string *jds, size_t *sp) {
  if (sp) *sp = jds->used;
  return jds->data;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
