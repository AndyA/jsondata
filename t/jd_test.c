/* jd_test.c */

#include "jsondata.h"
#include "jd_test.h"
#include "tap.h"

void jdt_dump(const char *label, jd_var *v) {
  jd_var json = JD_INIT;
  jd_to_json_pretty(&json, v);
  diag("%s: %s", label, jd_bytes(&json, NULL));
  jd_release(&json);
}

static int _is(jd_var *got, jd_var *want, const char *msg, va_list ap) {
  int rc;
  rc = test(jd_compare(got, want) == 0, msg, ap);
  if (!rc) {
    jdt_dump("wanted", want);
    jdt_dump("got", got);
  }
  return rc;
}

static int _isj(jd_var *got, jd_var *want, const char *msg, va_list ap) {
  jd_var vgot = JD_INIT;
  int rc;

  jd_to_json(&vgot, got);
  rc = _is(&vgot, want, msg, ap);
  jd_release(&vgot);
  return rc;
}

int jdt_is(jd_var *got, jd_var *want, const char *msg, ...) {
  va_list ap;
  jd_var vwant = JD_INIT;
  int rc;

  jd_to_json(&vwant, want);
  va_start(ap, msg);
  rc = _isj(got, &vwant, msg, ap);
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
  rc = _isj(got, &vwant, msg, ap);
  va_end(ap);
  jd_release(&vwant);
  return rc;
}

int jdt_is_string(jd_var *got, const char *want, const char *msg, ...) {
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

int jdt_throws(void (*func)(void *), void *ctx, const char *want, const char *msg, ...) {
  va_list ap;
  int rc;

  JD_BEGIN {
    JD_SV(caught, "");
    JD_SV(vwant, want);

    JD_BEGIN {
      func(ctx);
    }
    JD_CATCH(e) {
      jd_assign(caught, e);
    }
    JD_ENDCATCH

    va_start(ap, msg);
    rc = _is(caught, vwant, msg, ap);
    va_end(ap);
  }
  JD_END
  return rc;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
