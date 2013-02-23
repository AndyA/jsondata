/* jd_closure.c */

#include "jsondata.h"

jd_closure *jd_closure_new(jd_closure_func f) {
  jd_var ctx = JD_INIT;
  jd_closure *jdc = jd_alloc(sizeof(jd_closure));
  jdc->ctx = ctx;
  jdc->hdr.refs = 1;
  jdc->f = f;
  return jdc;
}

void jd_closure_retain(jd_closure *jdc) {
  jdc->hdr.refs++;
}

void jd_closure_free(jd_closure *jdc) {
  jd_release(&jdc->ctx);
  jd_free(jdc);
}

void jd_closure_release(jd_closure *jdc) {
  if (jdc->hdr.refs-- <= 1)
    jd_closure_free(jdc);
}

jd_var *jd_closure_context(jd_closure *jdc) {
  return &jdc->ctx;
}

jd_var *jd_closure_clone(jd_var *out, jd_closure *jdc, int deep) {
  jd_set_closure(out, jdc->f);
  if (deep) jd_clone(jd_context(out), &jdc->ctx, 1);
  else jd_assign(jd_context(out), &jdc->ctx);
  return out;
}

int jd_closure_call(jd_closure *jdc, jd_var *rv, jd_var *arg) {
  return jdc->f(rv, &jdc->ctx, arg);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
