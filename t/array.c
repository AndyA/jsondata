/* array.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static void test_sort(void) {
  jd_var ar = JD_INIT;

  jd_set_array(&ar, 10);
  jd_set_string(jd_push(&ar, 1), "Xylo");
  jd_set_string(jd_push(&ar, 1), "Glape");
  jd_set_string(jd_push(&ar, 1), "Cunic");
  jd_set_string(jd_push(&ar, 1), "Freeeb");
  jd_set_string(jd_push(&ar, 1), "Aardvark");

  jd_sort(&ar);

  jdt_is_json(&ar,
              "[\"Aardvark\",\"Cunic\",\"Freeeb\",\"Glape\",\"Xylo\"]",
              "string sort");

  jd_release(&ar);

}

static void test_join(void) {
  jd_var ar = JD_INIT, sep = JD_INIT, all = JD_INIT, want = JD_INIT;;

  jd_set_array(&ar, 10);
  jd_set_void(jd_push(&ar, 1));
  jd_set_bool(jd_push(&ar, 1), 0);
  jd_set_bool(jd_push(&ar, 1), 1);
  jd_set_int(jd_push(&ar, 1), 12345);
  jd_set_real(jd_push(&ar, 1), 1.25);
  jd_set_string(jd_push(&ar, 1), "foo");

  jd_set_string(&sep, ", ");
  jd_join(&all, &sep, &ar);
  jd_set_string(&want, "null, false, true, 12345, 1.25, foo");
  ok(jd_compare(&all, &want) == 0, "join on multiple types");

  jd_join(&all, NULL, &ar);
  jd_set_string(&want, "nullfalsetrue123451.25foo");
  ok(jd_compare(&all, &want) == 0, "join on multiple types, NULL sep");

  jd_release(&ar);
  jd_release(&sep);
  jd_release(&all);
  jd_release(&want);
}

static void test_basic(void) {
  jd_var ar = JD_INIT;
  jd_var v1 = JD_INIT, v2 = JD_INIT, v3 = JD_INIT;
  jd_var slot = JD_INIT;
  jd_var *tmp;
  size_t got;

  jd_set_string(&v1, "foo");
  jd_set_string(&v2, "bar");
  jd_set_string(&v3, "baz");

  jd_set_array(&ar,  2);

  jd_assign(jd_push(&ar, 1), &v1);
  jd_assign(jd_push(&ar, 1), &v2);
  jd_assign(jd_push(&ar, 1), &v3);

  jdt_is_json(&ar,
              "[\"foo\",\"bar\",\"baz\"]",
              "init");

  jd_assign(jd_get_idx(&ar, 1), jd_get_idx(&ar, 2));

  jdt_is_json(&ar,
              "[\"foo\",\"baz\",\"baz\"]",
              "copy 2->1");

  jd_assign(jd_push(&ar, 1), &v2);

  jdt_is_json(&ar,
              "[\"foo\",\"baz\",\"baz\",\"bar\"]",
              "push bar");

  is(jd_count(&ar), 4, "count");

  got = jd_shift(&ar, 1, NULL);
  is(got, 1, "shift count");

  jdt_is_json(&ar,
              "[\"baz\",\"baz\",\"bar\"]",
              "shift");

  is(jd_count(&ar), 3, "count");

  jd_assign(jd_unshift(&ar, 1), &v1);

  jdt_is_json(&ar,
              "[\"foo\",\"baz\",\"baz\",\"bar\"]",
              "unshift");

  is(jd_count(&ar), 4, "count");

  jd_append(&ar, &ar);

  jdt_is_json(&ar,
              "[\"foo\",\"baz\",\"baz\",\"bar\",\"foo\",\"baz\",\"baz\",\"bar\"]",
              "self append");

  is(jd_count(&ar), 8, "count");

  jd_pop(&ar, 1, &slot);
  jdt_is_json(&ar,
              "[\"foo\",\"baz\",\"baz\",\"bar\",\"foo\",\"baz\",\"baz\"]",
              "pop");
  jdt_is_json(&slot, "\"bar\"", "popped value");

  jd_remove(&ar, 1, 2, NULL);
  jdt_is_json(&ar,
              "[\"foo\",\"bar\",\"foo\",\"baz\",\"baz\"]",
              "remove");

  tmp = jd_insert(&ar, 1, 2);
  jd_set_bool(tmp++, 0);
  jd_set_int(tmp++, 123);

  jdt_is_json(&ar,
              "[\"foo\",false,123,\"bar\",\"foo\",\"baz\",\"baz\"]",
              "insert");

  jd_release(&ar);
  jd_release(&v1);
  jd_release(&v2);
  jd_release(&v3);
  jd_release(&slot);
}

static void throw_oob(void *ctx) {
  (void) ctx;
  scope {
    JD_VAR(ar);

    jd_set_array(ar, 3);
    jd_set_string(jd_push(ar, 1), "ZERO");
    jd_set_string(jd_push(ar, 1), "ONE");
    jd_get_idx(ar, 5);
  }
}

static void test_exceptions(void) {
  jdt_throws(throw_oob, NULL,
             "Array index 5 out of bounds (0..2)",
             "array index exception");
}

static void test_array_with(void) {
  scope {
    JD_VAR(ar);
    jd_set_array_with(ar, jd_nsv("Hello"), jd_niv(1234), NULL);
    jdt_is_json(ar, "[\"Hello\",1234]", "jd_set_array_with");
  }
}

static void test_no_scope(void) {
  jd_var ar = JD_INIT;
  jd_var val = JD_INIT;
  jd_set_array(&ar, 10);
  jd_set_string(&val, "Hello!");
  jd_assign(jd_push(&ar, 1), &val);
  jdt_is_json(jd_get_idx(&ar, 0), "\"Hello!\"", "jd_get_idx outside scope");
  jd_release(&ar);
  jd_release(&val);
}

static void test_flatten(void) {
  scope {
    jd_var *tmp = jd_nv();
    jdt_is_json(jd_flatten(tmp, jd_njv("[]")), "[]", "flatten empty");
    jdt_is_json(jd_flatten(tmp, jd_nsv("foo")), "\"foo\"", "flatten non-array");
    jdt_is_json(jd_flatten(tmp, jd_njv("[[[]]]")), "[]", "flatten deep empty");
    jdt_is_json(jd_flatten(tmp, jd_njv("[[[1]],2]")), "[1,2]", "flatten");
  }
}

void test_main(void) {
  test_basic();
  test_join();
  test_sort();
  test_exceptions();
  test_array_with();
  test_no_scope();
  test_flatten();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
