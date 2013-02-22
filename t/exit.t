/* map.t */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static int run_forked(void (*f)(void *), void *ctx, jd_var *out) {
  int outfd[2];
  pid_t pid;
  char buf[1024];
  int status;

  /* If we don't do this pending stdout/stderr output is emitted
   * by the forked instance.
   */
  fflush(stdout);
  fflush(stderr);

  jd_set_empty_string(out, 1000);
  if (pipe(outfd) < 0) jd_die("Can't open pipe: %m");
  pid = fork();
  if (pid < 0) jd_die("Can't fork: %m");

  if (pid == 0) {
    /* in child */
    close(outfd[0]);
    if (dup2(outfd[1], 1) < 0 || dup2(outfd[1], 2) < 0)
      jd_die("Can't switch stdout / stderr: %m");
    close(outfd[1]);
    f(ctx);
    exit(0);
  }

  close(outfd[1]);

  for (;;) {
    ssize_t got = read(outfd[0], buf, sizeof(buf));
    if (got < 0) jd_die("Read error: %m");
    if (got == 0) break;
    jd_append_bytes(out, buf, got);
  }

  close(outfd[0]);

  if (waitpid(pid, &status, 0) < 0)
    jd_die("waitpid failed: %m");

  return status;
}

static void check_exit(void (*f)(void *), void *ctx, const char *want, int want_status) {
  jd_var out = JD_INIT;
  int status;

  status = run_forked(f, ctx, &out);
  jdt_is_string(&out, want, "got exception");
  is(status, want_status, "exit status = %d", want_status);

  jd_release(&out);
}

static void uncaught(void *ctx) {
  jd_throw("Oops");
}

static void *nomem(size_t sz) {
  return NULL;
}

static void oom(void *ctx) {
  jd_alloc_hook = nomem;
  jd_var v = JD_INIT;
  jd_set_string(&v, "Boom!");
  jd_release(&v);
}

static void test_uncaught(void) {
  check_exit(uncaught, NULL, "Uncaught exception: Oops\n", 256);
}

static void test_oom(void) {
  check_exit(oom, NULL, "Fatal: Out of memory\n", 256);
}

void test_main(void) {
  test_uncaught();
  test_oom();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
