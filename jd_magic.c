/* jd_magic.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jsondata.h"
#include "jd_private.h"

static jd_var *get_magic(jd_string *s, unsigned mtype) {
  jd_dvar *m;

  for (m = s->magic; m; m = m->next)
    if (m->magic_type == mtype)
      return &m->v;

  m = jd_alloc(sizeof(jd_dvar));
  m->next = s->magic;
  m->magic_type = mtype;
  s->magic = m;
  return &m->v;
}

jd_var *jd__get_magic(jd_var *v, unsigned mtype) {
  return get_magic(jd__as_string(v), mtype);
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
