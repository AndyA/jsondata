/* jdtest.c */

#include <stdio.h>

#include "jsondata.h"

int main(void) {
  JD_TRY {
    JD_2VARS(a, b);
    jd_set_string(a, "This is A");
    jd_throw("Oops: a=%J", a);
    jd_set_bool(b, 1);
  } JD_CATCH(e) {
    jd_rethrow(e);
  }
  JD_END
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
