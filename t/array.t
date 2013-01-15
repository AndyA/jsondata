/* array.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jsondata.h"

static void check_ar(jd_var *ar, const char *expect) {
  jd_var sep, all;
  jd_set_string(&sep, "|");
  jd_join(&all, &sep, ar);
  if (!ok(!strcmp(all.v.s->data, expect), "array contents")) {
    diag("Expected: %s", expect);
    diag("Got:      %s", all.v.s->data);
  }
  jd_release(&all);
  jd_release(&sep);
}

static void t_array(void) {
  jd_var ar = JD_INIT, *v1, *v2, *v3;
  jd_set_array(&ar,  2);
  v1 = jd_push(&ar, 1);
  jd_set_string(v1, "foo");
  v2 = jd_push(&ar, 1);
  jd_set_string(v2, "bar");
  v3 = jd_push(&ar, 1);
  jd_assign(v3, v1);

  check_ar(&ar, "foo|bar|foo");

  jd_assign(jd_get(&ar, 1), jd_get(&ar, 2));

  check_ar(&ar, "foo|foo|foo");

  size_t got = jd_shift(&ar, 1, NULL);
  is(got, 1, "shift count");

  check_ar(&ar, "foo|foo");

  jd_release(&ar);
}

void test_main(void) {
  t_array();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
