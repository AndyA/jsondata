/* path.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jsondata.h"

void test_main(void) {
  jd_var m = JD_INIT;
  jd_var foo = JD_INIT;

  jd_set_hash(&m, 5);
  jd_set_string(&foo, "foo");

/*  jd_assign(jd_lv(&m, "$.slot.0.name"), &foo);*/

  jd_release(&m);
  jd_release(&foo);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
