/* setjmp-bug.c */

#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>

static jmp_buf jb;

static void test_setjmp1(void) {
  volatile void *p;
  if (!setjmp(jb)) {
    p = malloc(100);
    longjmp(jb, 1000);
  }
  printf("  p=%p (longjmp always)\n", p);
}

static void test_setjmp2(int t) {
  volatile void *p;
  if (!setjmp(jb)) {
    p = malloc(200);
    if (t) longjmp(jb, 2000);
  }
  printf("  p=%p (longjmp depends on arg)\n", p);
}

int main(void) {
  test_setjmp1();
  test_setjmp2(1);
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
