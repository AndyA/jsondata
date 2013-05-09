/* closure.t */

#include <stdio.h>

#include "framework.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static int tc1(jd_var *rv, jd_var *ctx, jd_var *arg) {
  (void) rv;
  (void) arg;
  if (ctx->type != INTEGER) jd_set_int(ctx, 0);
  jd_set_int(ctx, jd_get_int(ctx) + 1);
  return 0;
}

static void test_closure(void) {
  scope {
    JD_3VARS(cl1, cl2, args);
    jd_set_closure(cl1, tc1);
    jd_assign(cl2, cl1);
    jd_set_string(jd_context(cl1), "foo");
    jdt_is_json(jd_context(cl1), "\"foo\"", "initial context");
    jd_call(cl1, args);
    jdt_is_json(jd_context(cl1), "1", "one call");
    jd_call(cl2, NULL);
    jdt_is_json(jd_context(cl2), "2", "two calls");
    jd_call(cl1, args);
    jdt_is_json(jd_context(cl1), "3", "three calls");
  }
}

static int closure(jd_var *rv, jd_var *ctx, jd_var *arg) {
  (void) rv;
  (void) arg;
  jd_var *counter = jd_rv(ctx, "$.counter");
  jd_set_int(counter, jd_get_int(counter) + 1);
  return 0;
}

static void test_clone_deep(void) {
  scope {
    JD_3VARS(ctx, cl1, cl2);

    jd_set_hash(ctx, 1);
    jd_set_int(jd_lv(ctx, "$.counter"), 0);

    jd_set_closure(cl1, closure);
    jd_assign(jd_context(cl1), ctx);

    jd_clone(cl2, cl1, 1);
    jd_call(cl1, NULL);
    jd_call(cl2, NULL);

    jdt_is_json(jd_context(cl1), "{\"counter\":1}", "one call to original");
    jdt_is_json(jd_context(cl2), "{\"counter\":1}", "one call to clone");
  }
}

static void test_clone_shallow(void) {
  scope {
    JD_3VARS(ctx, cl1, cl2);

    jd_set_hash(ctx, 1);
    jd_set_int(jd_lv(ctx, "$.counter"), 0);

    jd_set_closure(cl1, closure);
    jd_assign(jd_context(cl1), ctx);

    jd_clone(cl2, cl1, 0);
    jd_call(cl1, NULL);
    jd_call(cl2, NULL);

    jdt_is_json(jd_context(cl1), "{\"counter\":2}", "two calls to original");
    jdt_is_json(jd_context(cl2), "{\"counter\":2}", "two calls to clone");
  }
}

void test_main(void) {
  test_closure();
  test_clone_deep();
  test_clone_shallow();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
