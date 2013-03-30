/* jsondata.h */

#ifndef JSONDATA_H_
#define JSONDATA_H_

#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>

typedef long jd_int;
#define JD_INT_FMT "%ld"
#define JD_STRTOINT strtol

#define JD_NORETURN __attribute__((noreturn))

#define JD_TYPE_LIST \
  /* simple */       \
  X(VOID)            \
  X(BOOL)            \
  X(INTEGER)         \
  X(REAL)            \
  X(STRING)          \
  /* complex */      \
  X(ARRAY)           \
  X(HASH)            \
  /* magic */        \
  X(CLOSURE)         \
  X(OBJECT)

#define X(x) x,
typedef enum {
  JD_TYPE_LIST
  MAXTYPE
} jd_type;
#undef X

#define JD_IS_SIMPLE(t)  ((t) <= STRING)
#define JD_IS_MAGIC(t)   ((t) >= CLOSURE)
#define JD_IS_COMPLEX(t) (!JD_IS_SIMPLE(t) && !JD_IS_MAGIC(t))

typedef struct _jd_var jd_var;
typedef struct _jd_hash_bucket jd_hash_bucket;
typedef struct _jd_closure jd_closure;

typedef int (*jd_closure_func)(jd_var *result, jd_var *context, jd_var *args);

typedef struct {
  unsigned refs;
} jd_ohdr;

/* jd_string is used both directly as a string and internally as a
 * growable buffer. jd_array contains a jd_string and can,
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
  void *alloca;
} jd_dvar;

typedef struct jd_activation {
  struct jd_activation *up;
  jmp_buf env;
  jd_dvar *vars;
  jd_var exception;
  const char *file;
  int line;
} jd_activation;

#define JD_GETEX(rec) \
  (rec ? &rec->exception : &jd_root_exception)

#define JD_SCOPE \
  for ( jd_activation *__jd_ar = jd_ar_push(__LINE__, __FILE__); \
        __jd_ar && !(setjmp(__jd_ar->env) && jd_rethrow(jd_catch(__jd_ar))); \
        jd_ar_up(__jd_ar), __jd_ar = NULL)

#define JD_TRY \
  do { \
    for ( jd_activation *__jd_ar = jd_ar_push(__LINE__, __FILE__); \
          __jd_ar && !(setjmp(__jd_ar->env) && jd_catch(__jd_ar)); \
          jd_ar_up(__jd_ar), __jd_ar = NULL)

#define JD_CATCH(e) \
  } while (0); \
  JD_SCOPE \
  for (jd_var *volatile e = JD_GETEX(jd_head->up); \
       e && e->type != VOID; \
       jd_release(e), e = NULL)

#define JD_RETURN(x) \
  return ({                   \
    __typeof__(x) _tmp = (x); \
    jd_ar_up(__jd_ar);        \
    _tmp;                     \
  })

#define JD_RETURN_VOID \
  do { jd_ar_up(__jd_ar); return; } while (0)

#define JD_VAR(x) \
  jd_var *x = jd_nv()

#define JD_2VARS(a, b) JD_VAR(a); JD_VAR(b)
#define JD_3VARS(a, b, c) JD_2VARS(a, b); JD_VAR(c)
#define JD_4VARS(a, b, c, d) JD_3VARS(a, b, c); JD_VAR(d)

#define JD_AV(n, v) jd_var *n = jd_nav(v)
#define JD_BV(n, v) jd_var *n = jd_nbv(v)
#define JD_CV(n, v) jd_var *n = jd_ncv(v)
#define JD_HV(n, v) jd_var *n = jd_nhv(v)
#define JD_IV(n, v) jd_var *n = jd_niv(v)
#define JD_JV(n, v) jd_var *n = jd_njv(v)
#define JD_RV(n, v) jd_var *n = jd_nrv(v)
#define JD_SV(n, v) jd_var *n = jd_nsv(v)

#define jd_throw_info(info, msg, ...) \
  jd_ar_throw_info(__FILE__, __LINE__, info, msg, ## __VA_ARGS__)
#define jd_throw(msg, ...) \
  jd_ar_throw(__FILE__, __LINE__, msg, ## __VA_ARGS__)

#define jd_head (*jd_head_tls())
#define jd_root_exception (*jd_root_exception_tls())

extern void *(*jd_alloc_hook)(size_t);
extern void (*jd_free_hook)(void *);

jd_var *jd_version(jd_var *out);
void jd_die(const char *msg, ...) JD_NORETURN;
void *jd_alloc(size_t sz);
void jd_free(void *m);
void jd_release(jd_var *v);
void jd_retain(jd_var *v);
jd_var *jd_assign(jd_var *dst, jd_var *src);
jd_var *jd_set_string(jd_var *v, const char *s);
jd_var *jd_set_bytes(jd_var *v, const void *s, size_t size);
jd_var *jd_set_empty_string(jd_var *v, size_t size);
jd_var *jd_set_array(jd_var *v, size_t size);
jd_var *jd_set_array_with(jd_var *v, ...);
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
int jd_rethrow(jd_var *e) JD_NORETURN;
void jd_ar_throw_info(const char *file, int line,
                      jd_var *info, const char *msg, ...) JD_NORETURN;
void jd_ar_throw(const char *file, int line,
                 const char *msg, ...) JD_NORETURN;
jd_var *jd_backtrace(jd_var *out);

jd_var *jd_nv(void);
jd_var *jd_nav(size_t v);
jd_var *jd_nbv(int v);
jd_var *jd_ncv(jd_closure_func v);
jd_var *jd_nhv(int v);
jd_var *jd_niv(jd_int v);
jd_var *jd_njv(const char *v);
jd_var *jd_nrv(double v);
jd_var *jd_nsv(const char *v);

void *jd_alloca(size_t sz);

jd_var *jd_map(jd_var *out, jd_var *func, jd_var *in);
jd_var *jd_grep(jd_var *out, jd_var *func, jd_var *in);
jd_var *jd_dmap(jd_var *out, jd_var *func, jd_var *in);
jd_var *jd_dgrep(jd_var *out, jd_var *func, jd_var *in);

jd_activation **jd_head_tls(void);
jd_var *jd_root_exception_tls(void);

#endif /*!JSONDATA_H_*/

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
