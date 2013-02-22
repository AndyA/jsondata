/* misc.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void check_numify(const char *in, const char *want, jd_type type) {
  JD_BEGIN {
    JD_2VARS(vin, vout);
    jd_set_string(vin, in);
    jd_numify(vout, vin);
    jdt_is_json(vout, want, "%s -> %s: value matches", in, want);
    is(vout->type, type, "%s -> %s: type matches", in, want);
  }
  JD_END
}

static void test_numify(void) {
  check_numify("true", "true", BOOL);
  check_numify("false", "false", BOOL);
  check_numify("null", "null", VOID);
  check_numify("10000000", "10000000", INTEGER);
  check_numify("1.25", "1.25", REAL);
}

static void test_numify_misc(void) {
  JD_BEGIN {
    JD_HV(a, 1);
    JD_IV(ac, 2);
    JD_VAR(an);

    jd_lv(a, "$.foo");
    jd_lv(a, "$.bar");
    jd_numify(an, a);
    jdt_is(an, ac, "numify hash -> count");
  }
  JD_END

  JD_BEGIN {
    JD_AV(a, 1);
    JD_IV(ac, 2);
    JD_VAR(an);

    jd_push(a, 2);
    jd_numify(an, a);
    jdt_is(an, ac, "numify array -> count");
  }
  JD_END
}

void test_main(void) {
  test_numify();
  test_numify_misc();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
