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
  jd_var *e = jd_head->up ? &jd_head->up->exception : &jd_root_exception;
  jd_assign(e, &jd_head->exception);
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
    jd_assign(&jd_head->exception, e);
    if (release) jd_release(e);
    longjmp(jd_head->env, 1);
  }
  else {
    jd_var *bt;
    fprintf(stderr, "Uncaught exception: %s\n",
            jd_bytes(jd_get_ks(e, "message", 0), NULL));
    if (bt = jd_get_ks(e, "backtrace", 0), bt) {
      size_t count = jd_count(bt);
      int i;
      for (i = 0; i < count; i++) {
        jd_var *slot = jd_get_idx(bt, i);
        fprintf(stderr, "  %sat %s:" JD_INT_FMT "\n",
                i ? "via context " : "",
                jd_bytes(jd_get_ks(slot, "file", 0), NULL),
                jd_get_int(jd_get_ks(slot, "line", 0)));
      }
    }
    if (release) jd_release(e);
    exit(1);
  }
}

int jd_rethrow(jd_var *e) {
  rethrow(e, 0);
}

static void bt_slot(jd_var *slot, const char *file, int line) {
  jd_set_string(jd_lv(slot, "$.file"), file);
  jd_set_int(jd_lv(slot, "$.line"), line);
}

jd_var *jd_backtrace(jd_var *out) {
  jd_activation *rec;
  jd_set_array(out, 40);

  for (rec = jd_head; rec; rec = rec->up)
    bt_slot(jd_push(out, 1), rec->file, rec->line);

  return out;
}

static void throw(const char *file, int line,
                  jd_var *info, const char *msg, va_list ap) JD_NORETURN;

static void throw(const char *file, int line,
                  jd_var *info, const char *msg, va_list ap) {
  jd_var e = JD_INIT;
  jd_var *bt = jd_lv(&e, "$.backtrace");
  jd_backtrace(bt);
  bt_slot(jd_unshift(bt, 1), file, line);
  jd_vprintf(jd_lv(&e, "$.message"), msg, ap);
  if (info) jd_assign(jd_lv(&e, "$.info"), info);
  rethrow(&e, 1);
}

void jd_ar_throw_info(const char *file, int line,
                      jd_var *info, const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  throw(file, line, info, msg, ap);
  va_end(ap);
}

void jd_ar_throw(const char *file, int line,
                 const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  throw(file, line, NULL, msg, ap);
  va_end(ap);
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
