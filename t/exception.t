/* exception.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void test_simple_throw(void) {
  JD_TRY {
    JD_2VARS(a, b);
    jd_set_string(a, "This is A");
    jd_throw("Oops: a=%J", a);
    jd_set_bool(b, 1);
  } JD_CATCH(e) {
    jdt_is_string(e, "Oops: a=\"This is A\"", "exception message matches");
    jd_release(e);
  }
  JD_END
}

static void go_deep(int depth) {
  JD_VAR(a);
  jd_printf(a, "depth=%d", depth);
  if (depth == 0)
    jd_throw("Reached the bottom");
  else
    go_deep(depth - 1);
}

static void test_deep_throw(void) {
  JD_TRY {
    go_deep(10);
  } JD_CATCH(e) {
    jdt_is_string(e, "Reached the bottom", "deep exception message matches");
    jd_release(e);
  }
  JD_END
}

static void nest_deep(int depth) {
  JD_TRY {
    JD_VAR(a);
    jd_printf(a, "depth=%d", depth);
    if (depth == 0)
      jd_throw("Reached the bottom");
    else
      nest_deep(depth - 1);
  } JD_CATCH(e) {
    jd_rethrow(e);
  }
  JD_END
}

static void test_deep_nest(void) {
  JD_TRY {
    nest_deep(10);
  } JD_CATCH(e) {
    jdt_is_string(e, "Reached the bottom", "nested scope exception message matches");
    jd_release(e);
  }
  JD_END
}

void test_main(void) {
  test_simple_throw();
  test_deep_throw();
  test_deep_nest();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
