/* array.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jsondata.h"

static void check_ar(jd_var *ar, const char *expect, const char *test) {
  jd_var sep, all;
  jd_set_string(&sep, "|");
  jd_join(&all, &sep, ar);
  if (!ok(!strcmp(all.v.s->data, expect), "array: %s", test)) {
    diag("Expected: %s", expect);
    diag("Got:      %s", all.v.s->data);
  }
  jd_release(&all);
  jd_release(&sep);
}

void test_main(void) {
  jd_var ar = JD_INIT;
  jd_var v1 = JD_INIT, v2 = JD_INIT, v3 = JD_INIT;
  size_t got;

  jd_set_string(&v1, "foo");
  jd_set_string(&v2, "bar");
  jd_set_string(&v3, "baz");

  jd_set_array(&ar,  2);

  jd_assign(jd_push(&ar, 1), &v1);
  jd_assign(jd_push(&ar, 1), &v2);
  jd_assign(jd_push(&ar, 1), &v3);

  check_ar(&ar, "foo|bar|baz", "init");

  jd_assign(jd_get(&ar, 1), jd_get(&ar, 2));

  check_ar(&ar, "foo|baz|baz", "copy 2->1");

  jd_assign(jd_push(&ar, 1), &v2);

  check_ar(&ar, "foo|baz|baz|bar", "push bar");

  got = jd_shift(&ar, 1, NULL);
  is(got, 1, "shift count");

  check_ar(&ar, "baz|baz|bar", "shift");

  jd_assign(jd_unshift(&ar, 1), &v1);

  check_ar(&ar, "foo|baz|baz|bar", "unshift");

  jd_append(&ar, &ar);

  check_ar(&ar, "foo|baz|baz|bar|foo|baz|baz|bar", "self append");

  jd_release(&ar);
  jd_release(&v1);
  jd_release(&v2);
  jd_release(&v3);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
