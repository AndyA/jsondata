/* string.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jsondata.h"

void test_main(void) {
  jd_var v1 = JD_INIT, v2 = JD_INIT;
  jd_set_string(&v1, "foo");
  jd_set_string(&v2, "bar");

  ok(jd_compare(&v1, &v1) == 0, "compare equal");
  ok(jd_compare(&v1, &v2) > 0 , "foo > bar");
  ok(jd_compare(&v2, &v1) <  0 , "bar < foo");

  jd_release(&v1);
  jd_release(&v2);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
