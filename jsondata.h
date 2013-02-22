/* jsondata.h */

#ifndef __JSONDATA_H
#define __JSONDATA_H

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>

typedef long long jd_int;

#define TD(x) x
typedef enum {
#include "jd_type.h"
  MAXTYPE
} jd_type;
#undef TD

#define JD_IS_COMPLEX(t) ((t) >= ARRAY)
#define JD_IS_SIMPLE(t)  (!JD_IS_COMPLEX(t))

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

typedef struct jd_dvar {
  struct jd_dvar *next;
  jd_var v;
} jd_dvar;

typedef struct jd_activation {
  struct jd_activation *up;
  jmp_buf env;
  jd_dvar *vars;
  const char *file;
  int line;
} jd_activation;

#define JD_BEGIN { \
    jd_activation *__jd_ar = jd_ar_push(__LINE__, __FILE__); \
    if (!setjmp(jd_head->env)) { if (1) do

#define JD_CATCH(e) \
  while (0); \
  jd_ar_up(__jd_ar); \
  } else { \
    jd_var *e = jd_catch(__jd_ar); \
    if (1)

#define JD_ENDCATCH }}

#define JD_END \
  JD_CATCH(e) { jd_rethrow(e); } JD_ENDCATCH

#define JD_RETURN(x) \
  do { jd_ar_up(__jd_ar); return (x); } while (0)

#define JD_VAR(x) \
  jd_var *x = jd_ar_var(jd_head)

#define JD_2VARS(a, b) JD_VAR(a); JD_VAR(b)
#define JD_3VARS(a, b, c) JD_2VARS(a, b); JD_VAR(c)
#define JD_4VARS(a, b, c, d) JD_3VARS(a, b, c); JD_VAR(d)

#define JD_AV(n, v) JD_VAR(n); jd_set_array(n, (v))
#define JD_BV(n, v) JD_VAR(n); jd_set_bool(n, (v))
#define JD_CV(n, v) JD_VAR(n); jd_set_closure(n, (v))
#define JD_HV(n, v) JD_VAR(n); jd_set_hash(n, (v))
#define JD_IV(n, v) JD_VAR(n); jd_set_int(n, (v))
#define JD_RV(n, v) JD_VAR(n); jd_set_real(n, (v))
#define JD_SV(n, v) JD_VAR(n); jd_set_string(n, (v))

extern __thread jd_activation *jd_head;
extern __thread jd_var jd_root_exception;

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
jd_var *jd_insert(jd_var *v, int idx, size_t count);
size_t jd_remove(jd_var *v, int idx, size_t count, jd_var *slot);
jd_var *jd_push(jd_var *v, size_t count);
size_t jd_pop(jd_var *v, size_t count, jd_var *slot);
jd_var *jd_unshift(jd_var *v, size_t count);
size_t jd_shift(jd_var *v, size_t count, jd_var *slot);
jd_var *jd_get_idx(jd_var *v, int idx);
jd_var *jd_get_key(jd_var *v, jd_var *key, int vivify);
jd_var *jd_get_ks(jd_var *v, const char *key, int vivify);
int jd_delete_key(jd_var *v, jd_var *key, jd_var *slot);
int jd_delete_ks(jd_var *v, const char *key, jd_var *slot);
size_t jd_length(jd_var *v);
size_t jd_count(jd_var *v);
jd_var *jd_append(jd_var *v, jd_var *v2);
jd_var *jd_append_bytes(jd_var *v, const void *b, size_t size);
jd_var *jd_join(jd_var *out, jd_var *sep, jd_var *ar);
int jd_compare(jd_var *a, jd_var *b);
unsigned long jd_hashcalc(jd_var *v);
jd_var *jd_keys(jd_var *out, jd_var *v);
jd_var *jd_stringify(jd_var *out, jd_var *v);
jd_var *jd_substr(jd_var *out, jd_var *v, int from, int len);
int jd_find(jd_var *haystack, jd_var *needle, int pos);
jd_var *jd_split(jd_var *out, jd_var *v, jd_var *sep);
jd_var *jd_numify(jd_var *out, jd_var *v);
int jd_test(jd_var *v);
jd_int jd_get_int(jd_var *v);
double jd_get_real(jd_var *v);
jd_var *jd_sortv(jd_var *v, int (*cmp)(jd_var *, jd_var *));
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
jd_var *jd_from_jsons(jd_var *out, const char *json);

jd_var *jd_vprintvf(jd_var *out, jd_var *fmt, va_list ap);
jd_var *jd_vprintf(jd_var *out, const char *fmt, va_list ap);
jd_var *jd_printvf(jd_var *out, jd_var *fmt, ...);
jd_var *jd_printf(jd_var *v, const char *fmt, ...);

jd_var *jd_context(jd_var *v);
jd_var *jd_eval(jd_var *cl, jd_var *rv, jd_var *arg);
void jd_call(jd_var *cl, jd_var *arg);
void *jd_ptr(jd_var *v);

jd_activation *jd_ar_push(int line, const char *file);
jd_activation *jd_ar_pop(void);
jd_var *jd_ar_var(jd_activation *rec);
void jd_ar_free(jd_activation *rec);
void jd_ar_up(jd_activation *rec);
jd_var *jd_catch(jd_activation *rec);
void jd_rethrow(jd_var *e);
void jd_throw(const char *msg, ...);
jd_var *jd_backtrace(jd_var *out);

jd_var *jd_map(jd_var *out, jd_var *func, jd_var *in);
jd_var *jd_grep(jd_var *out, jd_var *func, jd_var *in);
jd_var *jd_dmap(jd_var *out, jd_var *func, jd_var *in);
jd_var *jd_dgrep(jd_var *out, jd_var *func, jd_var *in);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
