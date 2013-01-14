/* jdtest.c */

#include <stdio.h>
#include "jsondata.h"

int main(void) {
  jd_var v1 = JD_INIT, v2 = JD_INIT;
  jd_set_string(&v1, "Hello, World");
  jd_assign(&v2, &v1);
  jd_release(&v1);
  printf("%s\n", v2.v.s->str);
  jd_release(&v2);
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
