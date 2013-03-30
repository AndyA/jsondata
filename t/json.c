/* json.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static void test_to_json(void) {
  jd_var vin = JD_INIT, vout = JD_INIT, vwant = JD_INIT;

  jd_set_array(&vin, 1);
  jd_set_string(jd_push(&vin, 1), "foo");
  jd_set_int(jd_push(&vin, 1), 12345);

  jd_set_string(&vwant, "[\"foo\",12345]");

  jd_to_json(&vout, &vin);
  ok(jd_compare(&vout, &vwant) == 0, "json");

  jd_set_string(jd_lv(&vin, "$.2.name"), "a hash");
  jd_set_int(jd_lv(&vin, "$.2.value"), 555);

  jd_set_string(&vwant, "[\"foo\",12345,{\"name\":\"a hash\",\"value\":555}]");

  jd_to_json(&vout, &vin);
  ok(jd_compare(&vout, &vwant) == 0, "json");

  jd_release(&vin);
  jd_release(&vout);
  jd_release(&vwant);
}

static void check_from_json(const char *json) {
  jd_var vout = JD_INIT;
  jd_from_jsons(&vout, json);
  jdt_is_json(&vout, json, "parse %s", json);
  jd_release(&vout);
}

static void test_from_json(void) {
  int i;
  static const char *json[] = {
    "\"\\u00FF\"",
    "[]",
    "\"foo\"",
    "{}",
    "[[],[]]",
    "[\"foo\",\"bar\"]",
    "{\"a\":\"b\"}",
    "{\"a\":{}}",
    "{\"a\":{},\"b\":{}}",
    "true",
    "false",
    "null",
    "[1,1.25,null,false,\"foo\",{\"k\":-1}]",
    "\"\\\"\\\"\"",
    "\"\\b\\t\\n\\f\\r\\\\\\\"\"",
    "\"\\b\\t\\n\\f\\r\\\\\\\"\\u00FF\"",
    "\"\\b\\t\\n\\f\\r\\\\\\\"\\u00FFFF\"",
    NULL
  };

  for (i = 0; json[i]; i++)
    check_from_json(json[i]);
}

static void throws(const char *json, const char *want, jd_int pos) {
  int thrown = 0;
  try {
    JD_VAR(out);
    jd_from_jsons(out, json);
  }
  catch (e) {
    jdt_is_string(jd_rv(e, "$.message"),
                  want, "parse \"%s\" throws \"%s\"", json, want);
    jd_int offset = jd_get_int(jd_rv(e, "$.info.offset"));
    if (!is(offset, pos, "position = " JD_INT_FMT, pos)) {
      diag("# json = %s", json);
      diag("# wanted " JD_INT_FMT, pos);
      diag("# got " JD_INT_FMT, offset);
    }
    jd_release(e);
    thrown = 1;
  }
  ok(thrown, "parsing \"%s\" throws exception", json);
}

static void test_exceptions(void) {
  throws("", "Syntax error", 0);
  throws("[1)", "Expected comma or closing bracket", 2);
  throws("{\"x\":1)", "Expected comma or closing brace", 6);
  throws("{\"x\")", "Missing colon", 4);
  throws("\"\\u\"", "Bad escape", 1);
  throws("triffic", "Expected true or false", 0);
  throws("nice", "Expected null", 0);
  throws("!!", "Syntax error", 0);
  throws("\"\\u0100\"", "Can't handle unicode", 1);
}

void test_main(void) {
  test_to_json();
  test_from_json();
  test_exceptions();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
