/* jdtest.c */

#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>

#include "jsondata.h"

typedef struct jd_activation {
  struct jd_activation *up;
  jmp_buf env;
  jd_var vars;
  const char *file;
  int line;
} jd_activation;

jd_activation *jd_head = NULL;
jd_var jd_root_exception = JD_INIT;

jd_activation *jd_ar_push(int line, const char *file) {
  jd_activation *rec = jd_alloc(sizeof(jd_activation));
  jd_set_array(&rec->vars, 10);
  rec->line = line;
  rec->file = file;
  rec->up = jd_head;
  return jd_head = rec;
}

jd_activation *jd_ar_pop(void) {
  jd_activation *rec = jd_head;
  jd_head = rec->up;
  return rec;
}

void jd_ar_free(jd_activation *rec) {
  jd_release(&rec->vars);
  jd_free(rec);
}

void jd_ar_up(void) {
  jd_ar_free(jd_ar_pop());
}

void jd_rethrow(jd_var *e) {
  if (jd_head) {
    jd_assign(jd_push(&jd_head->vars, 1), e);
    jd_release(e);
    longjmp(jd_head->env, 1);
  }
  else {
    fprintf(stderr, "Uncaught exception: %s\n", jd_bytes(e, NULL));
    jd_release(e);
    exit(1);
  }
}

void jd_throw(const char *msg, ...) {
  jd_var e = JD_INIT;
  va_list ap;
  va_start(ap, msg);
  jd_vprintf(&e, msg, ap);
  va_end(ap);
  jd_rethrow(&e);
}

#define JD_TRY \
  jd_ar_push(__LINE__, __FILE__); \
  if (!setjmp(jd_head->env)) { if (1) do

#define JD_CATCH(e) \
  while (0); \
  jd_ar_up(); \
  } else { \
    jd_var *e = jd_head && jd_head->up \
                ? jd_push(&jd_head->up->vars, 1) \
                : &jd_root_exception; \
    jd_pop(&jd_head->vars, 1, e); \
    jd_ar_up(); \
    if (1)

#define JD_END \
  }

#define JD_GUARD \
  JD_CATCH(e) { jd_rethrow(e); } JD_END

#define JD_VAR(x) \
  jd_var *x = jd_push(&jd_head->vars, 1)

#define JD_2VARS(a, b) JD_VAR(a); JD_VAR(b)
#define JD_3VARS(a, b, c) JD_2VARS(a, b); JD_VAR(c)
#define JD_4VARS(a, b, c, d) JD_3VARS(a, b, c); JD_VAR(d)

int main(void) {
  JD_TRY {
    JD_2VARS(a, b);
    jd_set_string(a, "This is A");
    jd_set_bool(b, 1);
  } JD_CATCH(e) {
    jd_rethrow(e);
  }
  JD_END
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
