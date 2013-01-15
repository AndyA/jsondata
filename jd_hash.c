/* jd_hash.c */

#include "jsondata.h"

jd_hash *jd_hash_new(size_t size) {
  jd_hash *jdh = jd_alloc(sizeof(jd_hash));
  jdh->b = jd_alloc(sizeof(jd_hash_bucket *) * size);
  jdh->size = size;
  jdh->hdr.refs = 1;
  return jdh;
}


jd_hash *jd_hash_retain(jd_hash *jdh) {
  jdh->hdr.refs++;
  return jdh;
}

void jd_hash_free(jd_hash *jdh) {
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
  jd_free(jdh);
}

jd_hash *jd_hash_release(jd_hash *jdh) {
  if (jdh->hdr.refs <= 1) {
    jd_hash_free(jdh);
    return NULL;
  }
  jdh->hdr.refs--;
  return jdh;
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
  }
  return &b->value;
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
  return 1;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
