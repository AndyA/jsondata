/* misc.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void check_numify(const char *in, const char *want, jd_type type) {
  jd_var vin = JD_INIT, vout = JD_INIT;

  jd_set_string(&vin, in);
  jd_numify(&vout, &vin);
  jdt_is_json(&vout, want, "%s -> %s: value matches", in, want);
  is(vout.type, type, "%s -> %s: type matches", in, want);

  jd_release(&vin);
  jd_release(&vout);
}

void test_main(void) {
  check_numify("true", "true", BOOL);
  check_numify("false", "false", BOOL);
  check_numify("null", "null", VOID);
  check_numify("10000000", "10000000", INTEGER);
  check_numify("1.25", "1.25", REAL);
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
