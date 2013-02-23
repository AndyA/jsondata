/* path.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void test_path(void) {
  jd_var m = JD_INIT;
  const char **pp;
  unsigned i;

  static const char *path[] = {
    "$.slot.0.name",
    "$.meta.info.title",
    "$.slot.1.name",
    "$.meta.info.date",
    NULL
  };

  static const char *notfound[] = {
    "$.not.found",
    "$.slot.2.name",
    "$.slot.0.title",
    NULL
  };

  static struct {
    const char *path;
    jd_type type;
  } types[] = {
    { "$.slot", ARRAY },
    { "$.slot.0", HASH },
    { "$", HASH }
  };

  jd_set_hash(&m, 5);

  for (pp = path; *pp; pp++) {
    jd_set_string(jd_lv(&m, *pp), *pp);
  }

  for (pp = path; *pp; pp++) {
    jd_var pv = JD_INIT;
    jd_set_string(&pv, *pp);
    ok(jd_compare(&pv, jd_rv(&m, *pp)) == 0, "Found %s", *pp);
    jd_release(&pv);
  }

  for (pp = notfound; *pp; pp++) {
    null(jd_rv(&m, *pp), "%s not found", *pp);
  }

  jdt_is(jd_rv(&m, "$.%s.%d.name", "slot", 1),
         jd_rv(&m, "$.slot.1.name"), "sprintf path");

  for (i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
    jd_var *v = jd_rv(&m, types[i].path);
    not_null(v, "%s: not null", types[i].path);
    if (!is(v->type, types[i].type, "%s: type %u", types[i].path, types[i].type)) {
      diag("Wanted: %d", types[i].type);
      diag("   Got: %d", v->type);
    }
  }

  jd_release(&m);
}

static void throw_unexpected(void *ctx) {
  JD_BEGIN {
    JD_VAR(x);
    jd_set_string(jd_lv(x, "$.hello"), "Hello, World");
    jd_set_int(jd_lv(x, "$.hello.index"), 123);
  }
  JD_END
}

static void test_exceptions(void) {
  jdt_throws(throw_unexpected, NULL,
             "Unexpected element in structure",
             "unexpected element in structure exception");
}

void test_main(void) {
  test_exceptions();
  test_path();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
