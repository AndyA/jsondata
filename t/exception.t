/* exception.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static void test_simple_throw(void) {
  try {
    JD_2VARS(a, b);
    jd_set_string(a, "This is A");
    jd_throw("Oops: a=%J", a);
    jd_set_bool(b, 1);
  }
  catch (e) {
    jdt_is_string(jd_rv(e, "$.message"),
                  "Oops: a=\"This is A\"",
                  "exception message matches");
    jd_release(e);
  }
}

static void test_throw_info(void) {
  try {
    JD_JV(info, "{\"name\":\"flake\"}");
    jd_throw_info(info, "Have some info");
  }
  catch (e) {
    jd_delete_ks(e, "backtrace", NULL);
    jdt_is_json(e,
                "{\"info\":{\"name\":\"flake\"},"
                "\"message\":\"Have some info\"}",
                "info intact");
    jd_release(e);
  }
}

static void go_deep(int depth) {
  JD_VAR(a);
  jd_printf(a, "depth=%d", depth);
  if (depth == 0)
    jd_throw("Reached the bottom");
  else
    go_deep(depth - 1);
}

static void test_deep_throw(void) {
  try {
    go_deep(10);
  }
  catch (e) {
    jdt_is_string(jd_rv(e, "$.message"),
                  "Reached the bottom",
                  "deep exception message matches");
    jd_release(e);
  }
}

static void nest_deep(int depth) {
  try {
    JD_VAR(a);
    jd_printf(a, "depth=%d", depth);
    if (depth == 0)
      jd_throw("Reached the bottom");
    else
      nest_deep(depth - 1);
  }
  catch (e) {
    jd_rethrow(e);
  }
}

static void test_deep_nest(void) {
  try {
    nest_deep(10);
  }
  catch (e) {
    jdt_is_string(jd_rv(e, "$.message"),
                  "Reached the bottom",
                  "nested scope exception message matches");
    jd_release(e);
  }
}

static void test_throw_in_catch(void) {
  int catch = 0, first = 0;

  try {
    JD_SV(a, "first");
    jd_throw("Throw from %V block", a);
  }
  catch (e) {
    try {
      JD_SV(a, "catch");
      jd_throw("Throw from %V block", a);
    }
    catch (e) {
      jdt_is_string(jd_rv(e, "$.message"),
                    "Throw from catch block", "got throw catch");
      catch ++;
    }
    jdt_is_string(jd_rv(e, "$.message"),
                  "Throw from first block", "got first catch");
    first++;
  }
  is(catch, 1, "catch block exception seen");
  is(first, 1, "first block exception seen");
}


static int fib(int x) {
  scope {
    JD_VAR(msg);
    jd_printf(msg, "fib(%d)", x);
    if (x < 2) JD_RETURN(1);
    JD_RETURN(fib(x - 1) + fib(x - 2));
  }
  return 0;
}

static void test_return(void) {
  scope {
    is(fib(5), 8, "fib 5");
  }
}

static void backtrace(jd_var *out, int depth) {
  scope {
    if (depth > 0)
      backtrace(out, depth - 1);
    else
      jd_backtrace(out);
  }
}

static void test_backtrace(void) {
  scope {
    JD_VAR(bt);
    backtrace(bt, 10);
    /*jdt_diag("bt=%lJ", bt);*/
    is(bt->type, ARRAY, "backtrace is array");
    size_t count = jd_count(bt);
    ok(count > 10 && count < 20, "backtrace has %ld elements", (unsigned long) count);
  }
}

/* Unfortunately the setjmp/longjmp that's used to implement try/catch
 * leaves any auto variables in the function's scope in an indeterminate
 * state. There's a good discussion of the problem here:
 *
 *  http://stackoverflow.com/questions/7271313/ \
 *    what-is-an-automatic-variable-in-this-setjmp-longjmp-context
 *  http://bit.ly/We5GaS
 *
 * Making block volatile appears not to work - possibly because the
 * volatility has to be cast away to pass it to jd_free().
 *
 * Caveat Emptor.
 */
static void *block = NULL;
static void cleanup(int throw) {
  try {
    block = jd_alloc(100);
    if (throw) jd_throw("Allocated block, freaked me out");
  }
  /*  jdt_diag("Cleanup, block=%p", block);*/
  jd_free(block);
  catch (e) jd_rethrow(e);
}

static void test_cleanup(void) {
  JD_VAR(exception);
  try {
    cleanup(1);
  }
  catch (e) {
    jd_assign(exception, e);
    jd_release(e);
  }
  is(exception->type, HASH, "exception fired");
}

static void auto_cleanup(int throw) {
  scope {
    char *block = jd_alloca(100);
    if (throw) jd_throw("Allocated block, freaked me out");
    strcpy(block, "Hello, World");
  }
}

static void test_auto_cleanup(void) {
  JD_VAR(exception);
  try {
    jd_alloca(200);
    auto_cleanup(0);
    auto_cleanup(1);
  }
  catch (e) {
    jd_assign(exception, e);
    jd_release(e);
  }
  is(exception->type, HASH, "exception fired");
}

void test_main(void) {
  scope {
    test_simple_throw();
    test_throw_info();
    test_deep_throw();
    test_deep_nest();
    test_throw_in_catch();
    test_return();
    test_backtrace();
    test_cleanup();
    test_auto_cleanup();
  }
}


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
