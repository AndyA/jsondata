/* jsondata.h */

#ifndef __JSONDATA_H
#define __JSONDATA_H

#include <stddef.h>

typedef long long jd_int;

typedef enum {
  VOID,
  BOOL,
  INTEGER,
  REAL,
  STRING,
  ARRAY,
  HASH
} jd_type;

typedef struct _jd_var jd_var;

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
  size_t seek;
} jd_array;

typedef struct {
  jd_string s;
} jd_hash;

struct _jd_var {
  jd_type type;
  union {
    int b;
    jd_int i;
    double r;
    jd_string *s;
    jd_array *a;
    jd_hash *h;
  } v;
};

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
jd_var *jd_set_empty_string(jd_var *v, size_t size);
jd_var *jd_set_array(jd_var *v, size_t size);
jd_string *jd_as_string(jd_var *v);
jd_array *jd_as_array(jd_var *v);
jd_var *jd_insert(jd_var *v, int idx, size_t count);
size_t jd_array_remove(jd_array *jda, int idx, size_t count, jd_var *slot);
jd_var *jd_push(jd_var *v, size_t count);
size_t jd_pop(jd_var *v, size_t count, jd_var *slot);
jd_var *jd_unshift(jd_var *v, size_t count);
size_t jd_shift(jd_var *v, size_t count, jd_var *slot);
size_t jd_array_pop(jd_array *jda, size_t count, jd_var *slot);
jd_var *jd_get(jd_var *v, int idx);
size_t jd_length(jd_var *v);
jd_var *jd_append(jd_var *v, jd_var *v2);
jd_var *jd_join(jd_var *out, jd_var *sep, jd_var *ar);
int jd_compare(jd_var *a, jd_var *b);

jd_string *jd_string_init(jd_string *jds, size_t size);
jd_string *jd_string_new(size_t size);
jd_string *jd_string_from(const char *s);
jd_string *jd_string_ensure(jd_string *jds, size_t size);
jd_string *jd_string_space(jd_string *jds, size_t minspace);
void jd_string_free(jd_string *jds);
jd_string *jd_string_retain(jd_string *jds);
jd_string *jd_string_release(jd_string *jds);
size_t jd_string_length(jd_string *jds);
jd_string *jd_string_append(jd_string *jds, jd_var *v);
int jd_string_compare(jd_string *jds, jd_var *v);

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

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
