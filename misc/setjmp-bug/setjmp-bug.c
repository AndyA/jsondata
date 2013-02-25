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
  printf("    p=%p (longjmp always)\n", p);
}

static void test_setjmp2(int t) {
  volatile void *p;
  if (!setjmp(jb)) {
    p = malloc(200);
    if (t) longjmp(jb, 2000);
  }
  printf("    p=%p (longjmp depends on constant true arg)\n", p);
}

static void test_setjmp3(int t) {
  volatile void *p;
  if (!setjmp(jb)) {
    p = malloc(300);
    if (t) longjmp(jb, 3000);
  }
  printf("    p=%p (longjmp depends on argc < 1000)\n", p);
}

int main(int argc, char *argv[]) {
  test_setjmp1();
  test_setjmp2(1);
  test_setjmp3(argc < 1000);
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
