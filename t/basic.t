/* basic.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void test_misc(void) {
  const char *hello = "Hello, World";
  jd_var v1 = JD_INIT, v2 = JD_INIT;
  jd_set_string(&v1, hello);
  jd_assign(&v2, &v1);
  jd_release(&v1);
  is(v2.v.s->hdr.refs, 1, "ref count");
  ok(!strcmp(v2.v.s->data, hello), "string");
  jd_release(&v2);
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
    jd_keys(tmp, ar);
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
             "Can't convert to a number",
             "can't convert to a number exception");
}

static void test_object(void) {
  JD_BEGIN {
    JD_VAR(o);
    jd_set_object(o, o, NULL);
    ok(jd_ptr(o) == o, "object stores pointer");
  }
  JD_END
}

void test_main(void) {
  test_misc();
  test_exceptions();
  test_object();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
