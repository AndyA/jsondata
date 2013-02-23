/* jd_exception.c */

#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "jd_private.h"
#include "jsondata.h"

__thread jd_activation *jd_head = NULL;
__thread jd_var jd_root_exception = JD_INIT;

jd_activation *jd_ar_push(int line, const char *file) {
  jd_activation *rec = jd_alloc(sizeof(jd_activation));
  rec->vars = NULL;
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

static void free_vars(jd_dvar *dv) {
  while (dv) {
    jd_dvar *next = dv->next;
    jd_release(&dv->v);
    jd_free(dv);
    dv = next;
  }
}

void jd_ar_free(jd_activation *rec) {
  free_vars(rec->vars);
  jd_free(rec);
}

void jd_ar_up(jd_activation *rec) {
  if (jd_head != rec) jd_die("Stack inbalance");
  jd_ar_free(jd_ar_pop());
}

jd_var *jd_catch(jd_activation *rec) {
  jd_dvar *ex = jd_head->vars;
  jd_var *e = &ex->v;
  jd_head->vars = ex->next;
  if (jd_head->up) {
    ex->next = jd_head->up->vars;
    jd_head->up->vars = ex;
  }
  else {
    jd_assign(&jd_root_exception, &ex->v);
    jd_release(&ex->v);
    jd_free(ex);
    e = &jd_root_exception;
  }
  jd_ar_up(rec);
  return e;
}

jd_var *jd_ar_var(jd_activation *rec) {
  jd_dvar *dv = jd_alloc(sizeof(jd_dvar));
  dv->next = rec->vars;
  rec->vars = dv;
  return &dv->v;
}

static void rethrow(jd_var *e, int release) JD_NORETURN;

static void rethrow(jd_var *e, int release) {
  if (jd_head) {
    /*    printf("throw %s at %s:%d\n", jd_bytes(e, NULL), jd_head->file, jd_head->line);*/
    jd_assign(jd_ar_var(jd_head), e);
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

jd_var *jd_backtrace(jd_var *out) {
  jd_activation *rec;
  jd_set_array(out, 40);

  for (rec = jd_head; rec; rec = rec->up) {
    jd_var ar = JD_INIT;
    jd_set_hash(&ar, 5);
    jd_set_int(jd_lv(&ar, "$.line"), rec->line);
    jd_set_string(jd_lv(&ar, "$.file"), rec->file);
    jd_assign(jd_push(out, 1), &ar);
    jd_release(&ar);
  }

  return out;
}

jd_var *jd_nv(void) {
  return jd_ar_var(jd_head);
}

#define MAKE_MAKER(n, t, f) \
  jd_var *jd_n ## n (t v) {  \
    jd_var *nv = jd_nv();   \
    f(nv, v);               \
    return nv;              \
  }

MAKE_MAKER(av, size_t, jd_set_array)
MAKE_MAKER(bv, int, jd_set_bool)
MAKE_MAKER(cv, jd_closure_func, jd_set_closure)
MAKE_MAKER(hv, int, jd_set_hash)
MAKE_MAKER(iv, jd_int, jd_set_int)
MAKE_MAKER(jv, const char *, jd_from_jsons)
MAKE_MAKER(rv, double, jd_set_real)
MAKE_MAKER(sv, const char *, jd_set_string)

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
