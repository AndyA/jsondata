/* misc.t */

#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static void test_retain_release(void) {
  int i;
  jd_var h = JD_INIT, s = JD_INIT;
  jd_set_string(&s, "Hello, World");
  jd_assign(jd_lv(&h, "$.hello"), &s);
  jd_release(&s);
  is(s.type, VOID, "release -> type == void");
  for (i = 0; i < 10; i++)
    jd_release(&s); /* make sure it's a nop */
  jdt_is_json(&h, "{\"hello\":\"Hello, World\"}", "data matches");
  jd_release(&h);
}

static void check_numify(const char *in, const char *want, jd_type type) {
  scope {
    JD_2VARS(vin, vout);
    jd_set_string(vin, in);
    jd_numify(vout, vin);
    jdt_is_json(vout, want, "%s -> %s: value matches", in, want);
    is(vout->type, type, "%s -> %s: type matches", in, want);
  }
}

static void test_numify(void) {
  check_numify("true", "true", BOOL);
  check_numify("false", "false", BOOL);
  check_numify("null", "null", VOID);
  check_numify("10000000", "10000000", INTEGER);
  check_numify("1.25", "1.25", REAL);
}

static void throw_null(void *ctx) {
  (void) ctx;
  jd_release(NULL);
}

static void throw_not_a_string(void *ctx) {
  (void) ctx;
  scope {
    JD_2VARS(ar, tmp);
    jd_set_array(ar, 1);
    jd_substr(tmp, ar, 0, 20);
  }
}

static void throw_not_an_array(void *ctx) {
  (void) ctx;
  scope {
    JD_VAR(ha);
    jd_set_hash(ha, 1);
    jd_get_idx(ha, 0);
  }
}

static void throw_not_a_hash(void *ctx) {
  (void) ctx;
  scope {
    JD_2VARS(ar, tmp);
    jd_set_array(ar, 1);
    jd_keys(ar, tmp);
  }
}

static void throw_not_a_closure(void *ctx) {
  (void) ctx;
  scope {
    JD_SV(s, "");
    jd_context(s);
  }
}

static void throw_not_an_object(void *ctx) {
  (void) ctx;
  scope {
    JD_SV(s, "");
    jd_ptr(s);
  }
}

static void throw_cant_append(void *ctx) {
  (void) ctx;
  scope {
    JD_IV(i, 0);
    JD_SV(s, "");
    jd_append(i, s);
  }
}

static void throw_cant_numify(void *ctx) {
  (void) ctx;
  scope {
    JD_VAR(o);
    jd_set_object(o, o, NULL);
    jd_numify(o, o);
  }
}

static void throw_cant_numify2(void *ctx) {
  (void) ctx;
  scope {
    JD_VAR(tmp);
    JD_SV(s, "123abc");
    jd_numify(tmp, s);
  }
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
  scope {
    JD_HV(a, 1);
    JD_IV(ac, 2);
    JD_VAR(an);

    jd_lv(a, "$.foo");
    jd_lv(a, "$.bar");
    jd_numify(an, a);
    jdt_is(an, ac, "numify hash -> count");
  }

  scope {
    JD_AV(a, 1);
    JD_IV(ac, 2);
    JD_VAR(an);

    jd_push(a, 2);
    jd_numify(an, a);
    jdt_is(an, ac, "numify array -> count");
  }
}

static void check_bool(const char **volatile expr, int volatile expect) {
  scope {
    JD_VAR(v);
    int i;

    for (i = 0; expr[i]; i++) {
      jd_from_jsons(v, expr[i]);
      is(jd_test(v), expect, "%s is %s", expr[i], expect ? "true" : "false");
    }
  }
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
  scope {
    JD_VAR(o);
    jd_set_object(o, o, NULL);
    ok(jd_ptr(o) == o, "object stores pointer");
  }
}

static void test_version(void) {
  scope {
    JD_VAR(v);
    jd_version(v);
    jdt_diag("Testing libjsondata %V", jd_rv(v, "$.version"));
  }
}

static void make_a_hash(jd_var *out) {
  jd_assign(jd_lv(out, "$.array"), jd_nav(0));
  jd_assign(jd_lv(out, "$.bool"), jd_nbv(1));
  jd_assign(jd_lv(out, "$.hash"), jd_nhv(0));
  jd_assign(jd_lv(out, "$.integer"), jd_niv(12345));
  jd_assign(jd_lv(out, "$.json"), jd_njv("{\"foo\":[1,2,3]}"));
  jd_assign(jd_lv(out, "$.real"), jd_nrv(0.25));
  jd_assign(jd_lv(out, "$.string"), jd_nsv("Hello, World"));
}

static void test_constructors(void) {
  scope {
    jd_var *hash = jd_nhv(MAXTYPE);
    make_a_hash(hash);
    /*    jdt_diag("hash: %J", hash);*/
    jdt_is_json(hash,
    "{\"array\":[],\"bool\":true,\"hash\":{},\"integer\":"
    "12345,\"json\":{\"foo\":[1,2,3]},\"real\":0.25,"
    "\"string\":\"Hello, World\"}",
    "constructed hash");
  }
}

static int promise_f(jd_var *result, jd_var *context, jd_var *arg) {
  (void) arg;
  jd_assign(result, context);
  return 1;
}

static void test_promise(void) {
  scope {
    jd_var *str = jd_nsv("Just a string");
    jdt_is_json(jd_promise(jd_nv(), str, NULL), "\"Just a string\"", "plain promise");
    jd_var *cl = jd_ncv(promise_f);
    jd_assign(jd_context(cl), jd_nsv("Promised"));
    jdt_is_json(jd_promise(jd_nv(), cl, NULL), "\"Promised\"", "closure promise");
  }
}

static void check_reverse(const char *in, const char *want) {
  scope {
    jd_var *src = jd_njv(in);
    jd_var *dst = jd_reverse(jd_nv(), src);
    jdt_is_json(dst, want, "reverse %s = %s (got %J)", in, want, dst);
  }
}

static void test_reverse(void) {
  check_reverse("1", "1");
  check_reverse("[1,2,3]", "[3,2,1]");
  check_reverse("[1,2]", "[2,1]");
  check_reverse("[1]", "[1]");
  check_reverse("[]", "[]");
  check_reverse("\"ABC\"", "\"CBA\"");
  check_reverse("\"AB\"", "\"BA\"");
  check_reverse("\"A\"", "\"A\"");
  check_reverse("\"\"", "\"\"");
  check_reverse("{\"one\":\"1\",\"two\":\"2\"}", "{\"1\":\"one\",\"2\":\"two\"}");
  check_reverse("{}", "{}");
}

void test_main(void) {
  test_version();
  test_retain_release();
  test_numify();
  test_numify_misc();
  test_test();
  test_exceptions();
  test_object();
  test_constructors();
  test_promise();
  test_reverse();
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
