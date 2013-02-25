/* setjmp-bug.c */

#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>

static jmp_buf jb;

static void test_setjmp(void) {
  volatile void *p;
  if (!setjmp(jb)) {
    p = malloc(100);
    longjmp(jb, 1000);
  }
  printf("p=%p\n", p);
}

int main(void) {
  test_setjmp();
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
