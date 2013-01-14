/* jd_string.c */

#include "jsondata.h"

#include <string.h>

jd_string *jd_string_new(size_t size) {
  jd_string *jds = jd_alloc(sizeof(jd_string));
  jds->str = jd_alloc(size);
  jds->size = size;
  jds->used = 0;
  jds->hdr.refs = 1; /* or maybe 0... */
  return jds;
}

jd_string *jd_string_from(const char *s) {
  size_t len = strlen(s) + 1;
  jd_string *jds = jd_string_new(len);
  memcpy(jds->str, s, len);
  jds->used = len;
  return jds;
}

jd_string *jd_string_ensure(jd_string *jds, size_t size) {
  if (jds->size < size) {
    char *nstr = jd_alloc(size);
    memcpy(nstr, jds->str, jds->used);
    jd_free(jds->str);
    jds->str = nstr;
  }
  return jds;
}

void jd_string_free(jd_string *jds) {
  jd_free(jds->str);
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

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
