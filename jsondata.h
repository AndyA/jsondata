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

struct _jd_var {
  jd_type type;
  union {
    int b;
    jd_int i;
    double r;
    jd_string *s;
    jd_array *a;
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
jd_var *jd_set_array(jd_var *v, size_t size);

jd_string *jd_string_init(jd_string *jds, size_t size);
jd_string *jd_string_new(size_t size);
jd_string *jd_string_from(const char *s);
jd_string *jd_string_ensure(jd_string *jds, size_t size);
jd_string *jd_string_space(jd_string *jds, size_t minspace);
void jd_string_free(jd_string *jds);
jd_string *jd_string_retain(jd_string *jds);
jd_string *jd_string_release(jd_string *jds);

jd_array *jd_array_new(size_t size);
jd_array *jd_array_retain(jd_array *jda);
jd_array *jd_array_release(jd_array *jds);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
