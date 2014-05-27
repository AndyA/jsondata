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

jd_string *jd__string_init(jd_string *jds, size_t size) {
  jds->data = jd_alloc(size);
  jds->size = size;
  jds->used = 0;
  jds->hdr.refs = 1; /* or maybe 0... */
  return jds;
}

jd_string *jd__string_new(size_t size) {
#ifdef ENABLE_DTRACE
  JSONDATA_STRING_NEW(size);
#endif
  jd_string *jds = jd_alloc(sizeof(jd_string));
  if (size == 0) size = 1;
  jd__string_init(jds, size);
  jds->used = 1; /* trailing \0 */
  return jds;
}

jd_string *jd__string_empty(jd_string *jds) {
  jds->used = 1;
  jds->data[0] = '\0';
  return jds;
}

jd_string *jd__string_from_bytes(const char *s, size_t size) {
  jd_string *jds = jd__string_new(size + 1);
  memcpy(jds->data, s, size);
  jds->data[size] = '\0';
  jds->used = size + 1;
  return jds;
}

jd_string *jd__string_from(const char *s) {
  return jd__string_from_bytes(s, strlen(s));
}

jd_string *jd__string_ensure(jd_string *jds, size_t size) {
  if (jds->size < size) {
    char *nstr = jd_alloc(size);
    memcpy(nstr, jds->data, jds->used);
    jd_free(jds->data);
    jds->data = nstr;
    jds->size = size;
  }
  return jds;
}

jd_string *jd__string_space(jd_string *jds, size_t minspace) {
  if (jds->size - jds->used < minspace) {
    size_t newsize = jds->used + minspace;
    if (newsize < jds->size * 2) newsize = jds->size * 2;
    return jd__string_ensure(jds, newsize);
  }
  return jds;
}

void jd__string_free(jd_string *jds) {
  jd__free_vars(jds->magic);
  jd_free(jds->data);
  jd_free(jds);
}

void jd__string_retain(jd_string *jds) {
  jds->hdr.refs++;
}

void jd__string_release(jd_string *jds) {
  if (jds->hdr.refs-- <= 1)
    jd__string_free(jds);
}

size_t jd__string_length(jd_string *jds) {
  return jds->used - 1;
}

jd_string *jd__string_append(jd_string *jds, jd_var *v) {
  jd_string *vs = jd__as_string(v);
  size_t len = jd__string_length(vs);
  jd__string_space(jds, len);
  memmove(jds->data + jds->used - 1, vs->data, len + 1);
  jds->used += len;
  return jds;
}

jd_string *jd__string_append_bytes(jd_string *jds, const void *b, size_t size) {
  jd__string_space(jds, size);
  memcpy(jds->data + jds->used - 1, b, size);
  jds->used += size;
  jds->data[jds->used - 1] = '\0';
  return jds;
}

int jd__string_compare(jd_string *jds, jd_var *v) {
  jd_string *vs = jd__as_string(v);
  size_t la = jd__string_length(jds);
  size_t lb = jd__string_length(vs);
  size_t lc = la < lb ? la : lb;
  int cmp = memcmp(jds->data, vs->data, lc);
  if (cmp) return cmp;
  return la < lb ? -1 : la > lb ? 1 : 0;
}

unsigned long jd__string_hashcalc(jd_string *jds, jd_type t) {
  unsigned long h = t;
  size_t len = jd__string_length(jds);
  unsigned i;
  for (i = 0; i < len; i++) {
    h = 31 * h + jds->data[i];
  }
  return h;
}

jd_var *jd__string_sub(jd_string *jds, int volatile from, int volatile len, jd_var *volatile out) {
  int sl = (int) jd__string_length(jds);

  if (from < 0) from += sl;
  if (len <= 0 || from < 0 || from >= sl) {
    jd_set_string(out, "");
    return out;
  }

  JD_SCOPE {
    JD_VAR(tmp);
    jd_string *jo;
    if (from + len > sl) len = sl - from;
    jd_set_empty_string(tmp, len);
    jo = jd__as_string(tmp);
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

int jd__string_find(jd_string *jds, jd_var *pat, int from) {
  jd_string *ps = jd__as_string(pat);
  size_t sl = jd__string_length(jds);
  size_t pl = jd__string_length(ps);
  if (from < 0) from += sl;
  if (from < 0 || from + pl > sl) return -1;
  const char *hit = memfind(jds->data + from, sl - from, ps->data, pl);
  if (hit) return hit - jds->data;
  return -1;
}

jd_var *jd__string_split(jd_string *jds, jd_var *pat, jd_var *out) {
  int pos;
  jd_set_array(out, 10);
  for (pos = 0; ;) {
    int hit = jd__string_find(jds, pat, pos);
    if (hit == -1) break;
    jd__string_sub(jds, pos, hit - pos, jd_push(out, 1));
    pos = hit + 1;
  }
  jd__string_sub(jds, pos, jd__string_length(jds) - pos, jd_push(out, 1));
  return out;
}

static int str_is(jd_string *jds, const char *s) {
  size_t jl = jd__string_length(jds);
  size_t sl = strlen(s);
  return jl == sl && memcmp(jds->data, s, jl) == 0;
}

jd_var *jd__string_numify(jd_string *jds, jd_var *out) {
  char *end;
  jd_int iv;
  double rv;
  int sl;

  if (str_is(jds, "true"))
    return jd_set_bool(out, 1);
  if (str_is(jds, "false"))
    return jd_set_bool(out, 0);
  if (str_is(jds, "null"))
    return jd_set_void(out);

  sl = (int) jd__string_length(jds);
  iv = (jd_int) JD_STRTOINT(jds->data, &end, 10);
  if (end - jds->data == sl)
    return jd_set_int(out, iv);

  /* If you happen to notice a valgrind error for this line see:
   *   http://sourceware.org/bugzilla/show_bug.cgi?id=12424
   */
  rv = strtod(jds->data, &end);
  if (end - jds->data == sl)
    return jd_set_real(out, rv);

  return NULL;
}

const char *jd__string_bytes(jd_string *jds, size_t *sp) {
  if (sp) *sp = jds->used;
  return jds->data;
}

jd_var *jd__string_reverse(jd_var *out, jd_string *jds) {
  size_t len = jds->used - 1;
  jd_set_empty_string(out, len);
  jd_string *os = jd__as_string(out);
  char *src = jds->data;
  char *dst = os->data;
  for (unsigned i = 0; i < len; i++)
    dst[len - i - 1] = src[i];
  dst[len] = '\0';
  os->used = len + 1;
  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
