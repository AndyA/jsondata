/* jd_hash.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jd_private.h"
#include "jsondata.h"

jd_hash *jd__hash_new(size_t size) {
  jd_hash *jdh = jd_alloc(sizeof(jd_hash));
  if (size < 20) size = 20;
  jdh->b = jd_alloc(sizeof(jd_hash_bucket *) * size);
  jdh->size = size;
  jdh->used = 0;
  jdh->hdr.refs = 1;
  return jdh;
}

void jd__hash_retain(jd_hash *jdh) {
  jdh->hdr.refs++;
}

static void free_contents(jd_hash *jdh) {
  unsigned i;
  for (i = 0; i < jdh->size; i++) {
    jd_hash_bucket *b, *next;
    for (b = jdh->b[i]; b; b = next) {
      next = b->next;
      jd_release(&b->key);
      jd_release(&b->value);
      jd_free(b);
    }
  }
  jd_free(jdh->b);
  jdh->b = NULL;
}

void jd_hash_free(jd_hash *jdh) {
  free_contents(jdh);
  jd_free(jdh);
}

void jd__hash_release(jd_hash *jdh) {
  if (jdh->hdr.refs-- <= 1)
    jd_hash_free(jdh);
}

size_t jd__hash_count(jd_hash *jdh) {
  return jdh->used;
}

static jd_hash_bucket *hash_find(jd_hash *jdh, jd_var *key, jd_hash_bucket ***ins) {
  unsigned long hash = jd_hashcalc(key);
  unsigned long slot = hash % jdh->size;
  *ins = &jdh->b[slot];
  for (;;) {
    jd_hash_bucket *b = **ins;
    if (!b) return b;
    if (0 == jd_compare(key, &b->key)) return b;
    *ins = &b->next;
  }
}

jd_var *jd__hash_get(jd_hash *jdh, jd_var *key, int vivify) {
  jd_hash_bucket **prev, *b;
  b = hash_find(jdh, key, &prev);
  if (!b) {
    if (!vivify) return NULL;
    b = jd_alloc(sizeof(jd_hash_bucket));
    jd_assign(&b->key, key);
    *prev = b;
    jdh->used++;
    /* conditional rehash; if we rehash we have to find the key again */
    if (jd__hash_maint(jdh))
      b = hash_find(jdh, key, &prev);
  }
  return &b->value;
}

jd_hash *jd__hash_rehash(jd_hash *jdh) {
  size_t count = jd__hash_count(jdh);
  jd_hash *tmp = jd__hash_new(count * 4);
  unsigned i;

  for (i = 0; i < jdh->size; i++) {
    jd_hash_bucket *b;
    for (b = jdh->b[i]; b; b = b->next) {
      jd_assign(jd__hash_get(tmp, &b->key, 1), &b->value);
    }
  }

  free_contents(jdh);
  jdh->b = tmp->b;
  jdh->size = tmp->size;
  jd_free(tmp);

  return jdh;
}

int jd__hash_maint(jd_hash *jdh) {
  if (jdh->used > jdh->size / 2) {
    jd__hash_rehash(jdh);
    return 1;
  }
  return 0;
}

int jd__hash_delete(jd_hash *jdh, jd_var *key, jd_var *slot) {
  jd_hash_bucket **prev, *b;
  b = hash_find(jdh, key, &prev);
  if (!b) return 0;
  if (slot) jd_assign(slot, &b->value);
  jd_release(&b->key);
  jd_release(&b->value);
  *prev = b->next;
  jd_free(b);
  jdh->used--;
  return 1;
}

jd_var *jd__hash_keys(jd_hash *jdh, jd_var *keys) {
  size_t count = jd__hash_count(jdh);
  unsigned i;

  jd_set_array(keys, count);
  jd_var *slot = jd_push(keys, count);

  for (i = 0; i < jdh->size; i++) {
    jd_hash_bucket *b;
    for (b = jdh->b[i]; b; b = b->next) {
      jd_assign(slot++, &b->key);
    }
  }
  return keys;
}

jd_var *jd__hash_merge(jd_var *out, jd_hash *jdh, int deep) {
  unsigned i;
  size_t count = jd__hash_count(jdh);
  jd_var keys = JD_INIT;
  jd__hash_keys(jdh, &keys);
  for (i = 0; i < count; i++) {
    jd_var *k = jd_get_idx(&keys, i);
    if (deep) jd_clone(jd_get_key(out, k, 1), jd__hash_get(jdh, k, 0), 1);
    else jd_assign(jd_get_key(out, k, 1), jd__hash_get(jdh, k, 0));
  }
  jd_release(&keys);
  return out;
}

jd_var *jd__hash_clone(jd_var *out, jd_hash *jdh, int deep) {
  jd_set_hash(out, jd__hash_count(jdh));
  return jd__hash_merge(out, jdh, deep);
}

jd_var *jd__hash_reverse(jd_var *out, jd_hash *jdh) {
  jd_var keys = JD_INIT;
  size_t count = jd__hash_count(jdh);

  jd__hash_keys(jdh, &keys);
  jd_set_hash(out, count * 2);

  for (unsigned i = 0; i < count; i++) {
    jd_var *k = jd_get_idx(&keys, i);
    jd_assign(jd_get_key(out, jd__hash_get(jdh, k, 0), 1), k);
  }

  jd_release(&keys);
  return out;
}

static int do_hash_compare(jd_hash *ha, jd_hash *hb, jd_var *keys) {
  size_t count = jd_count(keys);
  for (unsigned i = 0; i < count; i++) {
    jd_var *key = jd_get_idx(keys, i);
    jd_var *va = jd__hash_get(ha, key, 0);
    jd_var *vb = jd__hash_get(hb, key, 0);
    int cmp = jd_compare(va, vb);
    if (cmp) return cmp;
  }
  return 0;
}

int jd__hash_compare(jd_hash *ha, jd_hash *hb) {
  jd_var keys = JD_INIT;
  jd_var hm = JD_INIT;
  jd__hash_clone(&hm, ha, 0);
  jd__hash_merge(&hm, hb, 0);
  jd_keys(&keys, &hm);
  jd_release(&hm);
  jd_sort(&keys);
  int cmp = do_hash_compare(ha, hb, &keys);
  jd_release(&keys);
  return cmp;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
