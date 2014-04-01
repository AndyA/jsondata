/* jd_closure.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jsondata.h"

jd_closure *jd__closure_new(jd_closure_func f) {
  jd_var ctx = JD_INIT;
  jd_closure *jdc = jd_alloc(sizeof(jd_closure));
  jdc->ctx = ctx;
  jdc->hdr.refs = 1;
  jdc->f = f;
  return jdc;
}

void jd__closure_retain(jd_closure *jdc) {
  v->v.o.refs++;
}

void jd__closure_free(jd_closure *jdc) {
  jd_release(&jdc->ctx);
  jd_free(jdc);
}

void jd__closure_release(jd_closure *jdc) {
  if (v->v.o.refs-- <= 1)
    jd__closure_free(v->v.o.v.o);
}

jd_var *jd__closure_context(jd_closure *jdc) {
  return &jdc->ctx;
}

jd_var *jd__closure_clone(jd_var *out, jd_closure *jdc, int deep) {
  jd_set_closure(out, jdc->f);
  if (deep) jd_clone(jd_context(out), &jdc->ctx, 1);
  else jd_assign(jd_context(out), &jdc->ctx);
  return out;
}

int jd__closure_call(jd_closure *jdc, jd_var *rv, jd_var *arg) {
  return jdc->f(rv, &jdc->ctx, arg);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */

