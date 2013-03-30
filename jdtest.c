/* jdtest.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "jd_pretty.h"

int main(void) {
  try {
    JD_2VARS(a, b);
    jd_set_string(a, "This is A");
    jd_throw("Oops: a=%J", a);
    jd_set_bool(b, 1);
  }
  puts("cleanup");
  catch (e) jd_rethrow(e);

  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
