/* closure.t */

#include <stdio.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static int tc1(jd_var *rv, jd_var *ctx, jd_var *arg) {
  if (ctx->type != INTEGER) jd_set_int(ctx, 0);
  jd_set_int(ctx, jd_get_int(ctx) + 1);
  return 0;
}

void test_main(void) {
  jd_var cl = JD_INIT;
  jd_var args = JD_INIT;
  jd_set_closure(&cl, tc1);
  jd_set_string(jd_context(&cl), "foo");
  jdt_is_json(jd_context(&cl), "\"foo\"", "initial context");
  jd_call(&cl, &args);
  jdt_is_json(jd_context(&cl), "1", "one call");
  jd_call(&cl, &args);
  jdt_is_json(jd_context(&cl), "2", "two calls");
  jd_release(&cl);
  jd_release(&args);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
