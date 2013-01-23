/* jsondata.h */

#ifndef __JD_PRIVATE_H
#define __JD_PRIVATE_H

#include "jsondata.h"

#include <stddef.h>
#include <stdarg.h>

jd_string *jd_string_init(jd_string *jds, size_t size);
jd_string *jd_string_new(size_t size);
jd_string *jd_string_empty(jd_string *jds);
jd_string *jd_string_from_bytes(const char *s, size_t size);
jd_string *jd_string_from(const char *s);
jd_string *jd_string_ensure(jd_string *jds, size_t size);
jd_string *jd_string_space(jd_string *jds, size_t minspace);
void jd_string_free(jd_string *jds);
jd_string *jd_string_retain(jd_string *jds);
jd_string *jd_string_release(jd_string *jds);
size_t jd_string_length(jd_string *jds);
jd_string *jd_string_append(jd_string *jds, jd_var *v);
jd_string *jd_string_append_bytes(jd_string *jds, const void *b, size_t size);
int jd_string_compare(jd_string *jds, jd_var *v);
unsigned long jd_string_hashcalc(jd_string *jds);
jd_var *jd_string_sub(jd_string *jds, int from, int len, jd_var *out);
int jd_string_find(jd_string *jds, jd_var *pat, int from);
jd_var *jd_string_split(jd_string *jds, jd_var *pat, jd_var *out);
jd_var *jd_string_numify(jd_string *jds, jd_var *out);
const char *jd_string_bytes(jd_string *jds, size_t *sp);
jd_var *jd_context(jd_var *v);
jd_var *jd_eval(jd_var *cl, jd_var *rv, jd_var *arg);
void jd_call(jd_var *cl, jd_var *arg);
void *jd_ptr(jd_var *v);

jd_array *jd_array_new(size_t size);
jd_array *jd_array_retain(jd_array *jda);
jd_array *jd_array_release(jd_array *jds);
jd_var *jd_array_insert(jd_array *jda, int idx, size_t count);
size_t jd_array_remove(jd_array *jda, int idx, size_t count, jd_var *slot);
jd_var *jd_array_push(jd_array *jda, size_t count);
size_t jd_array_pop(jd_array *jda, size_t count, jd_var *slot);
jd_var *jd_array_unshift(jd_array *jda, size_t count);
size_t jd_array_shift(jd_array *jda, size_t count, jd_var *slot);
size_t jd_array_count(jd_array *jda);
jd_var *jd_array_get(jd_array *jda, int idx);
jd_var *jd_array_join(jd_var *out, jd_var *sep, jd_array *jda);
jd_array *jd_array_splice(jd_array *jda, int idx, jd_var *v);
jd_array *jd_array_append(jd_array *jda, jd_var *v);
jd_array *jd_array_sort(jd_array *jda);
jd_var *jd_array_clone(jd_var *out, jd_array *jda, int deep);

jd_hash *jd_hash_new(size_t size);
jd_hash *jd_hash_retain(jd_hash *jdh);
jd_hash *jd_hash_release(jd_hash *jdh);
size_t jd_hash_count(jd_hash *jdh);
jd_var *jd_hash_get(jd_hash *jdh, jd_var *key, int vivify);
int jd_hash_delete(jd_hash *jdh, jd_var *key, jd_var *slot);
jd_var *jd_hash_keys(jd_hash *jdh, jd_var *keys);
jd_var *jd_hash_merge(jd_var *out, jd_hash *jdh, int deep);
jd_var *jd_hash_clone(jd_var *out, jd_hash *jdh, int deep);
jd_hash *jd_hash_rehash(jd_hash *jdh);
int jd_hash_maint(jd_hash *jdh);

jd_closure *jd_closure_new(jd_closure_func f);
void jd_closure_free(jd_closure *jdc);
jd_closure *jd_closure_retain(jd_closure *jdc);
jd_closure *jd_closure_release(jd_closure *jdc);
jd_var *jd_closure_clone(jd_var *out, jd_closure *jdc, int deep);
jd_var *jd_closure_context(jd_closure *jdc);
int jd_closure_call(jd_closure *jdc, jd_var *rv, jd_var *arg);

jd_object *jd_object_new(void *o, void (*free)(void *));
jd_object *jd_object_retain(jd_object *jdo);
void jd_object_free(jd_object *jdo);
jd_object *jd_object_release(jd_object *jdo);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
