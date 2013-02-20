/* jd_exception.c */

#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "jd_private.h"
#include "jsondata.h"

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
  if (!rec) jd_die("Exception stack underrun");
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

static void rethrow(jd_var *e, int release) {
  if (jd_head) {
    /*    printf("throw %s at %s:%d\n", jd_bytes(e, NULL), jd_head->file, jd_head->line);*/
    jd_assign(jd_push(&jd_head->vars, 1), e);
    if (release) jd_release(e);
    longjmp(jd_head->env, 1);
  }
  else {
    fprintf(stderr, "Uncaught exception: %s\n", jd_bytes(e, NULL));
    if (release) jd_release(e);
    exit(1);
  }
}

void jd_rethrow(jd_var *e) {
  rethrow(e, 0);
}

void jd_throw(const char *msg, ...) {
  jd_var e = JD_INIT;
  va_list ap;
  va_start(ap, msg);
  jd_vprintf(&e, msg, ap);
  va_end(ap);
  rethrow(&e, 1);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
