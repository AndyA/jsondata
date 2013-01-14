/* jd_array.c */

#include "jsondata.h"

jd_array *jd_array_new(size_t size) {
  jd_array *jda = jd_alloc(sizeof(jd_array));
  jd_string_init(&jda->s, size * sizeof(jd_var));
  jda->seek = 0;
  return jda;
}

jd_array *jd_array_retain(jd_array *jda) {
  jd_string_retain(&jda->s);
  return jda;
}

jd_array *jd_array_release(jd_array *jda) {
  return jd_string_release(&jda->s) ? jda : NULL;
}

size_t jd_array_count(jd_array *jda) {
  return (jda->s.used - jda->seek) / sizeof(jd_var);
}

static unsigned check_idx(jd_array *jda, int idx) {
  size_t count = jd_array_count(jda);
  if (idx < 0) idx += count;
  if (idx < 0 || idx > count)
    jd_die("Array index %d out of bounds (0..%lu)", idx, (unsigned long) count);
  return (unsigned) idx;
}

jd_var *jd_insert(jd_array *jda, int idx, size_t count) {
  size_t ac = jd_array_count(jda);
  unsigned ix = check_idx(jda, idx);

  if (count) {
    size_t need = count * sizeof(jd_var);
    if (ix == ac) {
      jd_string_space(&jda->s, need);
      jd_var *v = (jd_var *)(jda->s.data + jda->s.used);
      jda->s.used += need;
      return v;
    }
    need -= jda->seek;
    /* TODO */
  }
  return NULL;
}

jd_var *jd_push(jd_array *jda, size_t count) {
  if (count) {
    size_t need = count * sizeof(jd_var);
    jd_var *v;
    jd_string_space(&jda->s, need);
    v = (jd_var *)(jda->s.data + jda->s.used);
    jda->s.used += need;
    return v;
  }
  return NULL;
}

jd_var *jd_unshift(jd_array *jda, size_t count) {
  return NULL;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
