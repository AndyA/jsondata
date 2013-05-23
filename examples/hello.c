/* hello.c */

#include <jd_pretty.h>

int main(void) {
  scope {
    jd_var *hello_world = jd_njv("{\"hello\":\"world\"}");
    jd_printf("The message is %lJ\n", hello_world);
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
