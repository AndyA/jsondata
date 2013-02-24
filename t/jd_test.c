/* jd_test.c */

#include "jd_pretty.h"
#include "jd_test.h"
#include "tap.h"

static int _is(jd_var *got, jd_var *want, const char *msg, va_list ap) {
  int rc;
  rc = test(jd_compare(got, want) == 0, msg, ap);
  if (!rc) {
    jdt_diag("wanted %lJ", want);
    jdt_diag("got %lJ", got);
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
  int rc = 0;

  scope {
    JD_VAR(caught);
    JD_SV(vwant, want);

    try {
      func(ctx);
    }
    catch(e) {
      jd_assign(caught, e);
    }

    va_start(ap, msg);
    rc = _is(jd_get_ks(caught, "message", 0), vwant, msg, ap);
    va_end(ap);
  }
  return rc;
}

void jdt_diag(const char *msg, ...) {
  scope {
    JD_VAR(vmsg);
    va_list ap;

    va_start(ap, msg);
    jd_vprintf(vmsg, msg, ap);
    va_end(ap);
    diag(jd_bytes(vmsg, NULL));
  }
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
