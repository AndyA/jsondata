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

static unsigned check_idx(jd_array *jda, int idx) {
  size_t count = jd_array_count(jda);
  if (idx < 0) idx += count;
  if (idx < 0 || idx >= count)
    jd_die("Array index %d out of bounds (0..%lu)", idx, (unsigned long) count);
  return (unsigned) idx;
}

#define BASE(jda) \
  ((jd_var *)((jda)->s.data + (jda)->seek))
#define ELT(jda, idx) \
  (BASE(jda) + (idx))

jd_var *jd_array_get(jd_array *jda, int idx) {
  return ELT(jda, check_idx(jda, idx));
}

jd_array *jd_array_release(jd_array *jda) {
  unsigned i;
  size_t ac;

  if (jd_string_release(&jda->s)) return jda;

  ac = jd_array_count(jda);
  for (i = 0; i < ac; i++) {
    jd_release(ELT(jda, i));
  }

  return NULL;
}

size_t jd_array_count(jd_array *jda) {
  return (jda->s.used - jda->seek) / sizeof(jd_var);
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

jd_var *jd_array_push(jd_array *jda, size_t count) {
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

jd_var *jd_array_join(jd_var *out, jd_var *sep, jd_array *jda) {
  size_t len = 0;
  size_t count = jd_array_count(jda);
  size_t slen = jd_length(sep);
  unsigned i;

  for (i = 0; i < count; i++) {
    if (i) len += slen;
    len += jd_length(ELT(jda, i));
  }

  jd_set_empty_string(out, len);

  for (i = 0; i < count; i++) {
    if (i) jd_append(out, sep);
    jd_append(out, ELT(jda, i));
  }

  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
