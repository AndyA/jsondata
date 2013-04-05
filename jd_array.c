/* jd_array.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "jd_private.h"
#include "jsondata.h"

#define BASE(jda) \
  ((jd_var *)((jda)->s.data))
#define ELT(jda, idx) \
  (BASE(jda) + (idx))

jd_array *jd__array_new(size_t size) {
  jd_array *jda = jd_alloc(sizeof(jd_array));
  jd__string_init(&jda->s, size * sizeof(jd_var));
  return jda;
}

void jd__array_retain(jd_array *jda) {
  jd__string_retain(&jda->s);
}

static void release(jd_array *jda, unsigned from, size_t count) {
  unsigned i;
  for (i = from; i < from + count; i++) jd_release(ELT(jda, i));
}

void jd__array_release(jd_array *jda) {
  if (jda->s.hdr.refs == 1)
    release(jda, 0, jd__array_count(jda));
  jd__string_release(&jda->s);
}

static unsigned cook_idx(int idx, size_t count, size_t max) {
  if (idx < 0) idx += count;
  if (idx < 0 || idx >= (int) max)
    jd_throw("Array index %d out of bounds (0..%lu)", idx, (unsigned long) max);
  return (unsigned) idx;
}

static unsigned check_idx(jd_array *jda, int idx) {
  size_t count = jd__array_count(jda);
  return cook_idx(idx, count, count);
}

static unsigned check_idx_open(jd_array *jda, int idx) {
  size_t count = jd__array_count(jda);
  return cook_idx(idx, count, count + 1);
}

jd_var *jd__array_get(jd_array *jda, int idx) {
  return ELT(jda, check_idx(jda, idx));
}

size_t jd__array_count(jd_array *jda) {
  return jda->s.used / sizeof(jd_var);
}

jd_var *jd__array_insert(jd_array *jda, int idx, size_t count) {
  unsigned ix = check_idx_open(jda, idx);
  size_t sz = jd__array_count(jda);
  size_t need = count * sizeof(jd_var);
  jd__string_space(&jda->s, need);
  memmove(ELT(jda, ix + count), ELT(jda, ix), (sz - ix) * sizeof(jd_var));
  memset(ELT(jda, ix), 0, count * sizeof(jd_var));
  jda->s.used += need;
  return ELT(jda, ix);
}

size_t jd__array_remove(jd_array *jda, int idx, size_t count, jd_var *slot) {
  unsigned ix = check_idx_open(jda, idx);
  size_t avail = jd__array_count(jda) - ix;
  if (count > avail) count = avail;
  if (slot) {
    unsigned i;
    for (i = 0; i < count; i++)
      jd_assign(slot++, ELT(jda, ix + i));
  }
  release(jda, ix, count);
  memmove(ELT(jda, ix), ELT(jda, ix + count), (avail - count) * sizeof(jd_var));
  jda->s.used -= count * sizeof(jd_var);

  return count;
}

jd_var *jd__array_push(jd_array *jda, size_t count) {
  return jd__array_insert(jda, jd__array_count(jda), count);
}

jd_var *jd__array_unshift(jd_array *jda, size_t count) {
  return jd__array_insert(jda, 0, count);
}

size_t jd__array_shift(jd_array *jda, size_t count, jd_var *slot) {
  return jd__array_remove(jda, 0, count, slot);
}

size_t jd__array_pop(jd_array *jda, size_t count, jd_var *slot) {
  return jd__array_remove(jda, -(int) count, count, slot);
}

static jd_var *array_stringify(jd_var *out, jd_array *jda) {
  size_t count = jd__array_count(jda);
  jd_set_array(out, count);
  unsigned i;

  for (i = 0; i < count; i++) {
    jd_stringify(jd_push(out, 1), ELT(jda, i));
  }

  return out;
}

static jd_var *array_join(jd_var *out, jd_var *sep, jd_array *jda) {
  size_t len = 0;
  size_t count = jd__array_count(jda);
  size_t slen = sep ? jd_length(sep) : 0;
  unsigned i;

  for (i = 0; i < count; i++) {
    if (i) len += slen;
    len += jd_length(ELT(jda, i));
  }

  jd_set_empty_string(out, len);

  for (i = 0; i < count; i++) {
    if (sep && i) jd_append(out, sep);
    jd_append(out, ELT(jda, i));
  }

  return out;
}

jd_var *jd__array_join(jd_var *out, jd_var *sep, jd_array *jda) {
  JD_SCOPE {
    JD_VAR(ar);
    array_stringify(ar, jda);
    array_join(out, sep, jd__as_array(ar));
  }
  return out;
}

jd_array *jd__array_splice(jd_array *jda, int idx, jd_var *v) {
  jd_array *va = jd__as_array(v);
  size_t count = jd__array_count(va);
  jd_var *slot = jd__array_insert(jda, idx, count);
  unsigned i;
  for (i = 0; i < count; i++) {
    jd_assign(&slot[i], ELT(va, i));
  }
  return jda;
}

jd_array *jd__array_append(jd_array *jda, jd_var *v) {
  return jd__array_splice(jda, jd__array_count(jda), v);
}

jd_array *jd__array_sort(jd_array *jda, int (*cmp)(jd_var *, jd_var *)) {
  qsort(jda->s.data, jd__array_count(jda), sizeof(jd_var),
        (int ( *)(const void *, const void *)) cmp);
  return jda;
}

jd_var *jd__array_clone(jd_var *out, jd_array *jda, int deep) {
  size_t count = jd__array_count(jda);
  jd_var *slot;
  unsigned i;

  jd_set_array(out, count);
  slot = jd_push(out, count);

  for (i = 0; i < count; i++) {
    if (deep) jd_clone(slot++, ELT(jda, i), 1);
    else jd_assign(slot++, ELT(jda, i));
  }

  return out;
}

jd_var *jd__array_reverse(jd_var *out, jd_array *jda) {
  size_t count = jd__array_count(jda);

  jd_set_array(out, count);
  jd_var *src = BASE(jda);
  jd_var *dst = jd_push(out, count);
  for (unsigned i = 0; i < count; i++)
    jd_assign(&dst[count - i - 1], &src[i]);

  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
