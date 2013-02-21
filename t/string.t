/* string.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void is_str(jd_var *v, const char *s, const char *msg) {
  jd_var sv = JD_INIT;
  jd_var vs = JD_INIT;
  jd_set_string(&sv, s);
  jd_stringify(&vs, v);
  ok(jd_compare(&sv, &vs) == 0, "%s", msg);
  jd_release(&sv);
  jd_release(&vs);
}

static void check_substr(const char *str, int from, int to, const char *want) {
  jd_var vs = JD_INIT, vw = JD_INIT, vsub = JD_INIT;

  jd_set_string(&vs, str);
  jd_set_string(&vw, want);

  jd_substr(&vsub, &vs, from, to);

  ok(jd_compare(&vw, &vsub) == 0, "substr(\"%s\", %d, %d) => \"%s\"",
     str, from, to, want);

  jd_release(&vs);
  jd_release(&vw);
  jd_release(&vsub);
}

static void check_find(const char *haystack, const char *needle, int pos, int want) {
  jd_var hs = JD_INIT, ns = JD_INIT;
  int got;

  jd_set_string(&hs, haystack);
  jd_set_string(&ns, needle);

  got = jd_find(&hs, &ns, pos);
  is(got, want, "find(\"%s\", \"%s\", %d) => %d", haystack, needle, pos, want);

  jd_release(&hs);
  jd_release(&ns);
}

static void check_split(const char *str, const char *sep, const char *want) {
  jd_var vstr = JD_INIT, vsep = JD_INIT, vwant = JD_INIT;
  jd_var vres = JD_INIT, vcomma = JD_INIT, vj = JD_INIT;

  jd_set_string(&vstr, str);
  jd_set_string(&vsep, sep);
  jd_set_string(&vwant, want);
  jd_set_string(&vcomma, ", ");

  jd_split(&vres, &vstr, &vsep);
  jd_join(&vj, &vcomma, &vres);

  ok(jd_compare(&vj, &vwant) == 0,
     "split(\"%s\", \"%s\") => %s", str, sep, want);

  jd_release(&vstr);
  jd_release(&vsep);
  jd_release(&vwant);
  jd_release(&vres);
  jd_release(&vcomma);
  jd_release(&vj);
}

static void test_substr(void) {
  check_substr("This is a stringy string", -6, 6, "string");
  check_substr("This is a stringy string", -6, 1000, "string");
  check_substr("abc", 0, 1, "a");
  check_substr("", -1000, 1000, "");
  check_substr("X", -1, 4, "X");
}

static void test_find(void) {
  check_find("X", "", 0, 0);
  check_find("X", "X", 0, 0);
  check_find("X", "X", 1, -1);
  check_find("XX", "X", 0, 0);
  check_find("XX", "XX", 0, 0);
  check_find("XX", "XXX", 0, -1);
  check_find("XXX", "X", 1, 1);
  check_find("XXX", "X", 2, 2);
  check_find("XXX", "X", 3, -1);
  check_find("abcdefgabcdefg", "a", 0, 0);
  check_find("abcdefgabcdefg", "b", 0, 1);
  check_find("abcdefgabcdefg", "g", 0, 6);
  check_find("abcdefgabcdefg", "ga", 0, 6);
  check_find("abcdefgabcdefg", "a", 1, 7);
  check_find("abcdefgabcdefg", "a", -10, 7);
}

static void test_split(void) {
  check_split("1,2,3", ",", "1, 2, 3");
  check_split("$.foo.0.bar", ".", "$, foo, 0, bar");
  check_split("Nothing to see here", "XXX", "Nothing to see here");
  check_split("", "X", "");
}

static void test_bytes(void) {
  int i;
  jd_var v1 = JD_INIT;
  const char *bytes = "bytes";

  jd_set_string(&v1, "string");
  for (i = 0; i < 3; i++) {
    jd_append_bytes(&v1, bytes, strlen(bytes));
  }

  jdt_is_json(&v1, "\"stringbytesbytesbytes\"", "append bytes");
  jd_release(&v1);
}

static void test_trim(void) {
  jd_var str = JD_INIT, out = JD_INIT;

  jd_set_string(&str, "\n\t TRIM THIS\n\n");
  jd_ltrim(&out, &str);
  jdt_is_json(&out, "\"TRIM THIS\\n\\n\"", "ltrim");

  jd_rtrim(&out, &str);
  jdt_is_json(&out, "\"\\n\\t TRIM THIS\"", "rtrim");

  jd_trim(&out, &str);
  jdt_is_json(&out, "\"TRIM THIS\"", "trim");

  jd_set_string(&str, "TRIM THIS");
  jd_trim(&out, &str);
  jdt_is_json(&out, "\"TRIM THIS\"", "trim (nop)");

  jd_set_string(&str, "");
  jd_ltrim(&out, &str);
  jdt_is_json(&out, "\"\"", "ltrim empty");

  jd_rtrim(&out, &str);
  jdt_is_json(&out, "\"\"", "rtrim empty");

  jd_trim(&out, &str);
  jdt_is_json(&out, "\"\"", "trim empty");

  jd_release(&str);
  jd_release(&out);
}

static void test_printf(void) {
  JD_BEGIN {
    JD_4VARS(v, p1, json, junk);

    jdt_is_string(jd_printf(v, "foo"), "foo", "printf");

    jdt_is_string(jd_printf(v, "%%"), "%", "printf %");

    jdt_is_string(jd_printf(v, "%d %i %o %u %x %X",
    1, 2, 100, 200, 300, 399),
    "1 2 144 200 12c 18F",
    "printf ints");

    jdt_is_string(jd_printf(v, "%s bar", "foo"), "foo bar", "printf char *");
    jdt_is_string(jd_printf(v, "%s %s", "foo", "bar"), "foo bar", "printf char *");

    jdt_is_string(jd_printf(v, "%s %-7s ", "foo", "bar"),
    "foo bar     ", "printf char * (padded)");

    jd_set_string(p1, "bar");
    jdt_is_string(jd_printf(v, "foo %V", p1), "foo bar", "printf jd_var *");

    jd_set_string(json, "{\"name\":\"foo\",\"value\":1.25}");
    jd_from_json(p1, json);
    jdt_is_string(jd_printf(v, "rec=%J", p1),
    "rec={\"name\":\"foo\",\"value\":1.25}",
    "printf json jd_var *");

    jdt_is_string(jd_printf(v, "rec=%lJ", p1),
    "rec={\n  \"name\": \"foo\",\n  \"value\": 1.25\n}",
    "printf pretty json jd_var *");

    jd_set_string(p1, "bar");
    jd_printvf(v, p1);
    ok(jd_bytes(v, NULL) == jd_bytes(p1, NULL), "printf format referenced");

    jd_set_string(junk, "JUNK ");
    while (jd_length(junk) < 20000) {
      jd_append(junk, junk);
    }
    jd_printf(v, "%s", jd_bytes(junk, NULL));
    jdt_is(v, junk, "printf long string");

  }
  JD_END
}

static void test_misc(void) {
  jd_var v1 = JD_INIT, v2 = JD_INIT;
  jd_set_string(&v1, "foo");
  jd_set_string(&v2, "bar");

  ok(jd_compare(&v1, &v1) == 0, "compare equal");
  ok(jd_compare(&v1, &v2) > 0 , "foo > bar");
  ok(jd_compare(&v2, &v1) <  0 , "bar < foo");

  jd_assign(&v2, &v1);
  jd_stringify(&v1, &v1);
  ok(jd_compare(&v1, &v2) == 0, "stringify string == nop");

  jd_set_int(&v1, 12345);
  is_str(&v1, "12345", "stringify integer");
  jd_set_bool(&v1, 1);
  is_str(&v1, "true", "stringify bool");
  jd_set_real(&v1, 1.25);
  is_str(&v1, "1.25", "stringify real");
  jd_set_void(&v1);
  is_str(&v1, "null", "stringify void");

  jd_release(&v1);
  jd_release(&v2);
}

void test_main(void) {
  test_misc();
  test_substr();
  test_find();
  test_split();
  test_bytes();
  test_printf();
  test_trim();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
