/* string.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
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

static void test_substr(const char *str, int from, int to, const char *want) {
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

static void test_find(const char *haystack, const char *needle, int pos, int want) {
  jd_var hs = JD_INIT, ns = JD_INIT;
  int got;

  jd_set_string(&hs, haystack);
  jd_set_string(&ns, needle);

  got = jd_find(&hs, &ns, pos);
  is(got, want, "find(\"%s\", \"%s\", %d) => %d", haystack, needle, pos, want);

  jd_release(&hs);
  jd_release(&ns);
}

static void test_split(const char *str, const char *sep, const char *want) {
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

void test_main(void) {
  jd_var v1 = JD_INIT, v2 = JD_INIT;
  jd_set_string(&v1, "foo");
  jd_set_string(&v2, "bar");

  ok(jd_compare(&v1, &v1) == 0, "compare equal");
  ok(jd_compare(&v1, &v2) > 0 , "foo > bar");
  ok(jd_compare(&v2, &v1) <  0 , "bar < foo");

  jd_printf(&v1, "Hello, %s", "World");
  jd_set_string(&v2, "Hello, World");
  ok(jd_compare(&v1, &v2) == 0, "printf");

  /*  jd_set_int(&v1, 12345);*/
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

  test_substr("This is a stringy string", -6, 6, "string");
  test_substr("This is a stringy string", -6, 1000, "string");
  test_substr("abc", 0, 1, "a");
  test_substr("", -1000, 1000, "");
  test_substr("X", -1, 4, "X");

  test_find("X", "", 0, 0);
  test_find("X", "X", 0, 0);
  test_find("X", "X", 1, -1);
  test_find("XX", "X", 0, 0);
  test_find("XX", "XX", 0, 0);
  test_find("XX", "XXX", 0, -1);
  test_find("XXX", "X", 1, 1);
  test_find("XXX", "X", 2, 2);
  test_find("XXX", "X", 3, -1);
  test_find("abcdefgabcdefg", "a", 0, 0);
  test_find("abcdefgabcdefg", "b", 0, 1);
  test_find("abcdefgabcdefg", "g", 0, 6);
  test_find("abcdefgabcdefg", "ga", 0, 6);
  test_find("abcdefgabcdefg", "a", 1, 7);
  test_find("abcdefgabcdefg", "a", -10, 7);

  test_split("1,2,3", ",", "1, 2, 3");
  test_split("$.foo.0.bar", ".", "$, foo, 0, bar");
  test_split("Nothing to see here", "XXX", "Nothing to see here");
  test_split("", "X", "");

  jd_release(&v1);
  jd_release(&v2);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
