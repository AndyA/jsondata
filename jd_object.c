/* jd_object.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jsondata.h"

jd_object *jd__object_new(void *o, void (*free)(void *)) {
  jd_object *jdo = jd_alloc(sizeof(jd_object));
  jdo->hdr.refs = 1;
  jdo->o = o;
  jdo->free = free;
  return jdo;
}

void jd__object_retain(jd_object *jdo) {
  jdo->hdr.refs++;
}

void jd__object_free(jd_object *jdo) {
  jd_free(jdo);
}

void jd__object_release(jd_object *jdo) {
  if (jdo->hdr.refs-- <= 1) {
    if (jdo->free) jdo->free(jdo->o);
    jd__object_free(jdo);
  }
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
