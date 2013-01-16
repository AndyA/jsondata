/* path.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jsondata.h"

void test_main(void) {
  jd_var vin = JD_INIT, vout = JD_INIT, vwant = JD_INIT;

  jd_set_array(&vin, 1);
  jd_set_string(jd_push(&vin, 1), "foo");
  jd_set_int(jd_push(&vin, 1), 12345);

  jd_set_string(&vwant, "[\"foo\",12345]");

  jd_to_json(&vout, &vin);
  ok(jd_compare(&vout, &vwant) == 0, "json");

  jd_release(&vin);
  jd_release(&vout);
  jd_release(&vwant);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
