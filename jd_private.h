/* jd_private.h */

#ifndef JD_PRIVATE_H_
#define JD_PRIVATE_H_

#include "jsondata.h"

#include <stddef.h>
#include <stdarg.h>

enum {
  MAGIC_NONE,
  MAGIC_PATH
};

jd_string *jd__string_init(jd_string *jds, size_t size);
jd_string *jd__string_new(size_t size);
jd_string *jd__string_empty(jd_string *jds);
jd_string *jd__string_from_bytes(const char *s, size_t size);
jd_string *jd__string_from(const char *s);
jd_string *jd__string_ensure(jd_string *jds, size_t size);
jd_string *jd__string_space(jd_string *jds, size_t minspace);
void jd__string_free(jd_string *jds);
void jd__string_retain(jd_string *jds);
void jd__string_release(jd_string *jds);
size_t jd__string_length(jd_string *jds);
jd_string *jd__string_append(jd_string *jds, jd_var *v);
jd_string *jd__string_append_bytes(jd_string *jds, const void *b, size_t size);
int jd__string_compare(jd_string *jds, jd_var *v);
unsigned long jd__string_hashcalc(jd_string *jds, jd_type t);
jd_var *jd__string_sub(jd_string *jds, int from, int len, jd_var *out);
int jd__string_find(jd_string *jds, jd_var *pat, int from);
jd_var *jd__string_split(jd_string *jds, jd_var *pat, jd_var *out);
jd_var *jd__string_numify(jd_string *jds, jd_var *out);
const char *jd__string_bytes(jd_string *jds, size_t *sp);

jd_string *jd__as_string(jd_var *v);
jd_array *jd__as_array(jd_var *v);
jd_hash *jd__as_hash(jd_var *v);
jd_closure *jd__as_closure(jd_var *v);
jd_object *jd__as_object(jd_var *v);

jd_array *jd__array_new(size_t size);
void jd__array_retain(jd_array *jda);
void jd__array_release(jd_array *jds);
jd_var *jd__array_insert(jd_array *jda, int idx, size_t count);
size_t jd__array_remove(jd_array *jda, int idx, size_t count, jd_var *slot);
jd_var *jd__array_push(jd_array *jda, size_t count);
size_t jd__array_pop(jd_array *jda, size_t count, jd_var *slot);
jd_var *jd__array_unshift(jd_array *jda, size_t count);
size_t jd__array_shift(jd_array *jda, size_t count, jd_var *slot);
size_t jd__array_count(jd_array *jda);
jd_var *jd__array_get(jd_array *jda, int idx);
jd_var *jd__array_join(jd_var *out, jd_var *sep, jd_array *jda);
jd_array *jd__array_splice(jd_array *jda, int idx, jd_var *v);
jd_array *jd__array_append(jd_array *jda, jd_var *v);
jd_array *jd__array_sort(jd_array *jda, int (*cmp)(jd_var *, jd_var *));
jd_var *jd__array_clone(jd_var *out, jd_array *jda, int deep);

jd_hash *jd__hash_new(size_t size);
void jd__hash_retain(jd_hash *jdh);
void jd__hash_release(jd_hash *jdh);
size_t jd__hash_count(jd_hash *jdh);
jd_var *jd__hash_get(jd_hash *jdh, jd_var *key, int vivify);
int jd__hash_delete(jd_hash *jdh, jd_var *key, jd_var *slot);
jd_var *jd__hash_keys(jd_hash *jdh, jd_var *keys);
jd_var *jd__hash_merge(jd_var *out, jd_hash *jdh, int deep);
jd_var *jd__hash_clone(jd_var *out, jd_hash *jdh, int deep);
jd_hash *jd__hash_rehash(jd_hash *jdh);
int jd__hash_maint(jd_hash *jdh);

jd_closure *jd__closure_new(jd_closure_func f);
void jd__closure_free(jd_closure *jdc);
void jd__closure_retain(jd_closure *jdc);
void jd__closure_release(jd_closure *jdc);
jd_var *jd__closure_clone(jd_var *out, jd_closure *jdc, int deep);
jd_var *jd__closure_context(jd_closure *jdc);
int jd__closure_call(jd_closure *jdc, jd_var *rv, jd_var *arg);

jd_object *jd__object_new(void *o, void (*free)(void *));
void jd__object_free(jd_object *jdo);
void jd__object_retain(jd_object *jdo);
void jd__object_release(jd_object *jdo);

void jd__free_vars(jd_dvar *dv);

jd_var *jd__get_magic(jd_var *v, unsigned mtype);

#endif /*!JD_PRIVATE_H_*/

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
