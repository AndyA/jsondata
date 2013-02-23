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

static void throw_null(void *ctx) {
  jd_release(NULL);
}

static void throw_not_a_string(void *ctx) {
  JD_BEGIN {
    JD_2VARS(ar, tmp);
    jd_set_array(ar, 1);
    jd_substr(tmp, ar, 0, 20);
  }
  JD_END
}

static void throw_not_an_array(void *ctx) {
  JD_BEGIN {
    JD_VAR(ha);
    jd_set_hash(ha, 1);
    jd_get_idx(ha, 0);
  }
  JD_END
}

static void throw_not_a_hash(void *ctx) {
  JD_BEGIN {
    JD_2VARS(ar, tmp);
    jd_set_array(ar, 1);
    jd_keys(ar, tmp);
  }
  JD_END
}

static void throw_not_a_closure(void *ctx) {
  JD_BEGIN {
    JD_SV(s, "");
    jd_context(s);
  }
  JD_END
}

static void throw_not_an_object(void *ctx) {
  JD_BEGIN {
    JD_SV(s, "");
    jd_ptr(s);
  }
  JD_END
}

static void throw_cant_append(void *ctx) {
  JD_BEGIN {
    JD_IV(i, 0);
    JD_SV(s, "");
    jd_append(i, s);
  }
  JD_END
}

static void throw_cant_numify(void *ctx) {
  JD_BEGIN {
    JD_VAR(o);
    jd_set_object(o, o, NULL);
    jd_numify(o, o);
  }
  JD_END
}

static void throw_cant_numify2(void *ctx) {
  JD_BEGIN {
    JD_VAR(tmp);
    JD_SV(s, "123abc");
    jd_numify(tmp, s);
  }
  JD_END
}

static void test_exceptions(void) {
  jdt_throws(throw_null, NULL,
             "Null pointer",
             "null pointer exception");
  jdt_throws(throw_not_a_string, NULL,
             "Not a string",
             "not a string exception");
  jdt_throws(throw_not_an_array, NULL,
             "Not an array",
             "not an array exception");
  jdt_throws(throw_not_a_hash, NULL,
             "Not a hash",
             "not a hash exception");
  jdt_throws(throw_not_a_closure, NULL,
             "Not a closure",
             "not a closure exception");
  jdt_throws(throw_not_an_object, NULL,
             "Not an object",
             "not an object exception");
  jdt_throws(throw_cant_append, NULL,
             "Can't append",
             "can't append exception");
  jdt_throws(throw_cant_numify, NULL,
             "Can't numify",
             "can't numify exception");
  jdt_throws(throw_cant_numify2, NULL,
             "Can't convert to a number",
             "can't convert to a number exception");
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
      jd_from_jsons(v, expr[i]);
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

static void test_object(void) {
  JD_BEGIN {
    JD_VAR(o);
    jd_set_object(o, o, NULL);
    ok(jd_ptr(o) == o, "object stores pointer");
  }
  JD_END
}

static void test_version(void) {
  JD_BEGIN {
    JD_VAR(v);
    jd_version(v);
    jdt_diag("jsondata %V (%V)", jd_rv(v, "$.version"), jd_rv(v, "$.date"));
  }
  JD_END
}

void test_main(void) {
  test_numify();
  test_numify_misc();
  test_test();
  test_exceptions();
  test_object();
  test_version();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
