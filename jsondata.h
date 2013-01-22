/* jsondata.h */

#ifndef __JSONDATA_H
#define __JSONDATA_H

#include <stddef.h>
#include <stdarg.h>

typedef long long jd_int;

typedef enum {
  VOID,
  BOOL,
  INTEGER,
  REAL,
  STRING,
  ARRAY,
  HASH,
  CLOSURE,
  OBJECT
} jd_type;

typedef struct _jd_var jd_var;
typedef struct _jd_hash_bucket jd_hash_bucket;
typedef struct _jd_closure jd_closure;

typedef int (*jd_closure_func)(jd_var *result, jd_var *context, jd_var *args);

typedef struct {
  unsigned refs;
} jd_ohdr;

/* jd_string is used both directly as a string and internally as a
 * growable buffer. jd_array and jd_hash start with a jd_string and can,
 * for operations that affect only the string part, be cast to
 * jd_string.
 */
typedef struct {
  jd_ohdr hdr;
  size_t size, used;
  char *data;
} jd_string;

typedef struct {
  jd_string s;
} jd_array;

typedef struct {
  jd_ohdr hdr;
  size_t size;
  size_t used;
  jd_hash_bucket **b;
} jd_hash;

typedef struct {
  jd_ohdr hdr;
  void *o;
  void (*free)(void *);
} jd_object;

struct _jd_var {
  jd_type type;
  union {
    int b;
    jd_int i;
    double r;
    jd_string *s;
    jd_array *a;
    jd_hash *h;
    jd_closure *c;
    jd_object *o;
  } v;
};

struct _jd_closure {
  jd_ohdr hdr;
  jd_var ctx;
  jd_closure_func f;
};

struct _jd_hash_bucket {
  jd_var key;
  jd_var value;
  struct _jd_hash_bucket *next;
};

typedef struct {
  /* TODO */
} jd_path_context;

#define JD_INIT { .type = VOID }

extern void *(*jd_alloc_hook)(size_t);
extern void (*jd_free_hook)(void *);

void jd_die(const char *msg, ...);
void *jd_alloc(size_t sz);
void jd_free(void *m);
void jd_release(jd_var *v);
void jd_retain(jd_var *v);
jd_var *jd_assign(jd_var *dst, jd_var *src);
jd_var *jd_set_string(jd_var *v, const char *s);
jd_var *jd_set_bytes(jd_var *v, const void *s, size_t size);
jd_var *jd_set_empty_string(jd_var *v, size_t size);
jd_var *jd_set_array(jd_var *v, size_t size);
jd_var *jd_set_hash(jd_var *v, size_t size);
jd_var *jd_set_closure(jd_var *v, jd_closure_func f);
jd_var *jd_set_object(jd_var *v, void *o, void (*free)(void *));
jd_var *jd_set_int(jd_var *v, jd_int i);
jd_var *jd_set_real(jd_var *v, double r);
jd_var *jd_set_bool(jd_var *v, int b);
jd_var *jd_set_void(jd_var *v);
jd_string *jd_as_string(jd_var *v);
jd_array *jd_as_array(jd_var *v);
jd_hash *jd_as_hash(jd_var *v);
jd_closure *jd_as_closure(jd_var *v);
jd_object *jd_as_object(jd_var *v);
jd_var *jd_insert(jd_var *v, int idx, size_t count);
size_t jd_remove(jd_var *v, int idx, size_t count, jd_var *slot);
jd_var *jd_push(jd_var *v, size_t count);
size_t jd_pop(jd_var *v, size_t count, jd_var *slot);
jd_var *jd_unshift(jd_var *v, size_t count);
size_t jd_shift(jd_var *v, size_t count, jd_var *slot);
jd_var *jd_get_idx(jd_var *v, int idx);
jd_var *jd_get_key(jd_var *v, jd_var *key, int vivify);
int jd_delete_key(jd_var *v, jd_var *key, jd_var *slot);
size_t jd_length(jd_var *v);
size_t jd_count(jd_var *v);
jd_var *jd_append(jd_var *v, jd_var *v2);
jd_var *jd_append_bytes(jd_var *v, const void *b, size_t size);
jd_var *jd_join(jd_var *out, jd_var *sep, jd_var *ar);
int jd_compare(jd_var *a, jd_var *b);
unsigned long jd_hashcalc(jd_var *v);
jd_var *jd_keys(jd_var *v, jd_var *keys);
jd_var *jd_stringify(jd_var *out, jd_var *v);
jd_var *jd_substr(jd_var *out, jd_var *v, int from, int len);
int jd_find(jd_var *haystack, jd_var *needle, int pos);
jd_var *jd_split(jd_var *out, jd_var *v, jd_var *sep);
jd_var *jd_numify(jd_var *out, jd_var *v);
jd_int jd_get_int(jd_var *v);
double jd_get_real(jd_var *v);
jd_var *jd_sort(jd_var *v);
const char *jd_bytes(jd_var *v, size_t *sp);
jd_var *jd_merge(jd_var *out, jd_var *v, int deep);
jd_var *jd_clone(jd_var *out, jd_var *v, int deep);
jd_var *jd_ltrim(jd_var *out, jd_var *v);
jd_var *jd_rtrim(jd_var *out, jd_var *v);
jd_var *jd_trim(jd_var *out, jd_var *v);

jd_var *jd_get_context(jd_var *root, jd_var *path,
                       jd_path_context *ctx, int vivify);
jd_var *jd_lv(jd_var *root, const char *path, ...);
jd_var *jd_rv(jd_var *root, const char *path, ...);

jd_var *jd_to_json(jd_var *out, jd_var *v);
jd_var *jd_to_json_pretty(jd_var *out, jd_var *v);
jd_var *jd_from_json(jd_var *out, jd_var *json);

jd_var *jd_vprintvf(jd_var *out, jd_var *fmt, va_list ap);
jd_var *jd_vprintf(jd_var *out, const char *fmt, va_list ap);
jd_var *jd_printvf(jd_var *out, jd_var *fmt, ...);
jd_var *jd_printf(jd_var *v, const char *fmt, ...);

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
