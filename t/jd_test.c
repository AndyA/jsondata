/* jd_test.c */

#include "jsondata.h"
#include "jd_test.h"
#include "tap.h"

void jdt_dump(const char *label, jd_var *v) {
  jd_var json = JD_INIT;
  jd_to_json(&json, v);
  diag("%s: %s", label, json.v.s->data);
  jd_release(&json);
}

static int _is(jd_var *got, jd_var *want, const char *msg, va_list ap) {
  jd_var vgot = JD_INIT;
  int rc;

  jd_to_json(&vgot, got);
  rc = test(jd_compare(&vgot, want) == 0, msg, ap);
  if (!rc) {
    jdt_dump("wanted", want);
    jdt_dump("got", &vgot);
  }
  jd_release(&vgot);
  return rc;
}

int jdt_is(jd_var *got, jd_var *want, const char *msg, ...) {
  va_list ap;
  jd_var vwant = JD_INIT;
  int rc;

  jd_to_json(&vwant, want);
  va_start(ap, msg);
  rc = _is(got, &vwant, msg, ap);
  va_end(ap);
  jd_release(&vwant);
  return rc;
}

int jdt_is_json(jd_var *got, const char *want, const char *msg, ...) {
  va_list ap;
  jd_var vwant = JD_INIT;
  int rc;

  jd_set_string(&vwant, want);
  va_start(ap, msg);
  rc = _is(got, &vwant, msg, ap);
  va_end(ap);
  jd_release(&vwant);
  return rc;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
