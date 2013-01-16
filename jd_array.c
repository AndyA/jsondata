/* jd_array.c */

#include <string.h>

#include "jsondata.h"

#define BASE(jda) \
  ((jd_var *)((jda)->s.data + (jda)->seek))
#define ELT(jda, idx) \
  (BASE(jda) + (idx))

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

static void release(jd_array *jda, unsigned from, size_t count) {
  unsigned i;
  for (i = from; i < from + count; i++) jd_release(ELT(jda, i));
}

jd_array *jd_array_release(jd_array *jda) {
  if (jda->s.hdr.refs == 1)
    release(jda, 0, jd_array_count(jda));
  return jd_string_release(&jda->s) ? jda : NULL;
}

static unsigned cook_idx(int idx, size_t count, size_t max) {
  if (idx < 0) idx += count;
  if (idx < 0 || idx >= max)
    jd_die("Array index %d out of bounds (0..%lu)", idx, (unsigned long) max);
  return (unsigned) idx;
}

static unsigned check_idx(jd_array *jda, int idx) {
  size_t count = jd_array_count(jda);
  return cook_idx(idx, count, count);
}

static unsigned check_idx_open(jd_array *jda, int idx) {
  size_t count = jd_array_count(jda);
  return cook_idx(idx, count, count + 1);
}

jd_var *jd_array_get(jd_array *jda, int idx) {
  return ELT(jda, check_idx(jda, idx));
}

size_t jd_array_count(jd_array *jda) {
  return (jda->s.used - jda->seek) / sizeof(jd_var);
}

jd_var *jd_array_insert(jd_array *jda, int idx, size_t count) {
  unsigned ix = check_idx_open(jda, idx);
  size_t sz = jd_array_count(jda);
  size_t need = count * sizeof(jd_var);
  jd_string_space(&jda->s, need);
  memmove(ELT(jda, ix + count), ELT(jda, ix), (sz - ix) * sizeof(jd_var));
  memset(ELT(jda, ix), 0, count * sizeof(jd_var));
  jda->s.used += need;
  return ELT(jda, ix);
}

size_t jd_array_remove(jd_array *jda, int idx, size_t count, jd_var *slot) {
  unsigned ix = check_idx_open(jda, idx);
  size_t avail = jd_array_count(jda) - ix;
  if (count > avail) count = avail;
  if (slot) {
    unsigned i;
    for (i = 0; i < count; i++)
      jd_assign(slot++, ELT(jda, ix + i));
  }
  release(jda, ix, count);
  if (idx == 0)
    jda->seek += count * sizeof(jd_var);
  else
    memmove(ELT(jda, ix), ELT(jda, ix + count), (avail - count) * sizeof(jd_var));
  return count;
}

jd_var *jd_array_push(jd_array *jda, size_t count) {
  return jd_array_insert(jda, jd_array_count(jda), count);
}

jd_var *jd_array_unshift(jd_array *jda, size_t count) {
  return jd_array_insert(jda, 0, count);
}

size_t jd_array_shift(jd_array *jda, size_t count, jd_var *slot) {
  return jd_array_remove(jda, 0, count, slot);
}

size_t jd_array_pop(jd_array *jda, size_t count, jd_var *slot) {
  return jd_array_remove(jda, -(int) count, count, slot);
}

static jd_var *array_stringify(jd_var *out, jd_array *jda) {
  size_t count = jd_array_count(jda);
  jd_set_array(out, count);
  unsigned i;

  for (i = 0; i < count; i++) {
    jd_stringify(jd_push(out, 1), ELT(jda, i));
  }

  return out;
}

static jd_var *array_join(jd_var *out, jd_var *sep, jd_array *jda) {
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

jd_var *jd_array_join(jd_var *out, jd_var *sep, jd_array *jda) {
  jd_var ar = JD_INIT;
  array_stringify(&ar, jda);
  array_join(out, sep, jd_as_array(&ar));
  jd_release(&ar);
  return out;
}

jd_array *jd_array_splice(jd_array *jda, int idx, jd_var *v) {
  jd_array *va = jd_as_array(v);
  size_t count = jd_array_count(va);
  jd_var *slot = jd_array_insert(jda, idx, count);
  unsigned i;
  for (i = 0; i < count; i++) {
    jd_assign(&slot[i], ELT(va, i));
  }
  return jda;
}

jd_array *jd_array_append(jd_array *jda, jd_var *v) {
  return jd_array_splice(jda, jd_array_count(jda), v);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
