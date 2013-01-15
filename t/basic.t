/* basic.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jsondata.h"

void test_main(void) {
  const char *hello = "Hello, World";
  jd_var v1 = JD_INIT, v2 = JD_INIT;
  jd_set_string(&v1, hello);
  jd_assign(&v2, &v1);
  jd_release(&v1);
  is(v2.v.s->hdr.refs, 1, "ref count");
  ok(!strcmp(v2.v.s->data, hello), "string");
  jd_release(&v2);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
