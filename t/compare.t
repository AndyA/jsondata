/* compare.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void check_sort(const char *in, const char *want) {
  JD_BEGIN {
    JD_SV(vin, in);
    JD_SV(vwant, want);
    JD_2VARS(jin, jwant);
    jd_from_json(jin, vin);
    jd_from_json(jwant, vwant);
    jd_sort(jin);
    jdt_is(jin, jwant, "%s sorts as %s", in, want);
  }
  JD_CATCH(e) {
    jdt_diag("Exception: %V", e);
    fail("Exception: %s", jd_bytes(e, NULL));
  }
  JD_ENDCATCH
}

void test_main(void) {
  JD_BEGIN {
    check_sort("[\"c\",\"b\",\"a\"]", "[\"a\",\"b\",\"c\"]");
    check_sort("[3,2,1]", "[1,2,3]");
    check_sort("[3.25,2.25,1.25]", "[1.25,2.25,3.25]");
    check_sort("[4,2,3.25,1.25,2,1.25]", "[1.25,1.25,2,2,3.25,4]");
    check_sort("[\"pi\",4,3.25,2,1.25]", "[1.25,2,3.25,4,\"pi\"]");
    check_sort("[true,false,\"pi\",4,3.25,2,1.25]",
    "[false,true,1.25,2,3.25,4,\"pi\"]");
  } JD_END
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
