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

  jd_release(&v1);
  jd_release(&v2);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
