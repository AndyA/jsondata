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

typedef struct {
  jd_ohdr hdr;
  size_t size, used;
  char *str;
} jd_string;

typedef struct {
} jd_array;

struct _jd_var {
  jd_type type;
  union {
    int b;
    jd_int i;
    double r;
    jd_string *s;
  } v;
};

#define JD_INIT { .type = VOID }

void *jd_alloc(size_t sz);
void jd_free(void *m);
void jd_release(jd_var *v);
void jd_retain(jd_var *v);
jd_var *jd_assign(jd_var *dst, jd_var *src);
jd_var *jd_set_string(jd_var *v, const char *s);

jd_string *jd_string_new(size_t size);
jd_string *jd_string_from(const char *s);
jd_string *jd_string_ensure(jd_string *jds, size_t size);
void jd_string_free(jd_string *jds);
jd_string *jd_string_retain(jd_string *jds);
jd_string *jd_string_release(jd_string *jds);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
