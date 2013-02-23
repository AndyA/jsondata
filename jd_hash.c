/* jd_hash.c */

#include "jd_private.h"
#include "jsondata.h"

jd_hash *jd_hash_new(size_t size) {
  jd_hash *jdh = jd_alloc(sizeof(jd_hash));
  jdh->b = jd_alloc(sizeof(jd_hash_bucket *) * size);
  jdh->size = size;
  jdh->used = 0;
  jdh->hdr.refs = 1;
  return jdh;
}

void jd_hash_retain(jd_hash *jdh) {
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

void jd_hash_release(jd_hash *jdh) {
  if (jdh->hdr.refs-- <= 1)
    jd_hash_free(jdh);
}

size_t jd_hash_count(jd_hash *jdh) {
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

jd_var *jd_hash_get(jd_hash *jdh, jd_var *key, int vivify) {
  jd_hash_bucket **prev, *b;
  b = hash_find(jdh, key, &prev);
  if (!b) {
    if (!vivify) return NULL;
    b = jd_alloc(sizeof(jd_hash_bucket));
    jd_assign(&b->key, key);
    *prev = b;
    jdh->used++;
    /* conditional rehash; if we rehash we have to find the key again */
    if (jd_hash_maint(jdh))
      b = hash_find(jdh, key, &prev);
  }
  return &b->value;
}

jd_hash *jd_hash_rehash(jd_hash *jdh) {
  size_t count = jd_hash_count(jdh);
  jd_hash *tmp = jd_hash_new(count * 2);
  unsigned i;

  for (i = 0; i < jdh->size; i++) {
    jd_hash_bucket *b;
    for (b = jdh->b[i]; b; b = b->next) {
      jd_assign(jd_hash_get(tmp, &b->key, 1), &b->value);
    }
  }

  free_contents(jdh);
  jdh->b = tmp->b;
  jdh->size = tmp->size;
  jd_free(tmp);

  return jdh;
}

int jd_hash_maint(jd_hash *jdh) {
  if (jdh->used > jdh->size / 2) {
    jd_hash_rehash(jdh);
    return 1;
  }
  return 0;
}

int jd_hash_delete(jd_hash *jdh, jd_var *key, jd_var *slot) {
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

jd_var *jd_hash_keys(jd_hash *jdh, jd_var *keys) {
  size_t count = jd_hash_count(jdh);
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

jd_var *jd_hash_merge(jd_var *out, jd_hash *jdh, int deep) {
  JD_BEGIN {
    unsigned i;
    size_t count = jd_hash_count(jdh);
    JD_VAR(keys);

    jd_hash_keys(jdh, keys);
    for (i = 0; i < count; i++) {
      jd_var *k = jd_get_idx(keys, i);
      if (deep) jd_clone(jd_get_key(out, k, 1), jd_hash_get(jdh, k, 0), 1);
      else jd_assign(jd_get_key(out, k, 1), jd_hash_get(jdh, k, 0));
    }

  }
  JD_END
  return out;
}

jd_var *jd_hash_clone(jd_var *out, jd_hash *jdh, int deep) {
  jd_set_hash(out, jd_hash_count(jdh));
  return jd_hash_merge(out, jdh, deep);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
