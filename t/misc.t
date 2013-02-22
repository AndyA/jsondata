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

static void throw_cant_numify(void *ctx) {
  JD_BEGIN {
    JD_VAR(o);
    jd_set_object(o, o, NULL);
    jd_numify(o, o);
  }
  JD_END
}

static void test_exceptions(void) {
  jdt_throws(throw_cant_numify, NULL,
             "Can't numify",
             "can't numify exception");
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

static void check_bool(const char *expr[], int expect) {
  JD_BEGIN {
    JD_VAR(v);
    int i;

    for (i = 0; expr[i]; i++) {
      jd_from_jsonc(v, expr[i]);
      is(jd_test(v), expect, "%s is %s", expr[i], expect ? "true" : "false");
    }
  }
  JD_END
}

static void test_test(void) {
  static const char *truths[] = {
    "true",
    "0.1",
    "1",
    "-1",
    "\"0\"",
    "\"false\"",
    "[1]",
    "{\"0\":0}",
    "[null]",
    NULL
  };

  static const char *lies[] = {
    "false",
    "null",
    "0.0",
    "0",
    "\"\"",
    "[]",
    "{}",
    NULL
  };

  check_bool(truths, 1);
  check_bool(lies, 0);
}

void test_main(void) {
  test_numify();
  test_numify_misc();
  test_test();
  test_exceptions();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
