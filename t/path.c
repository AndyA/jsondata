/* path.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"
#include "jd_path.h"

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
    jd_var *vp = jd_rv(&m, *pp);
    if (!null(vp, "%s not found", *pp))
      jdt_diag("Expected NULL, got %lJ", vp);
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
  (void) ctx;
  scope {
    JD_VAR(x);
    jd_set_string(jd_lv(x, "$.hello"), "Hello, World");
    jd_set_int(jd_lv(x, "$.hello.index"), 123);
  }
}

static void throw_bad_path(void *ctx) {
  (void) ctx;
  scope {
    JD_VAR(x);
    jd_set_string(jd_lv(x, "@.hello"), "Hello, World");
    jdt_diag("No exception thrown, x=%J", x);
  }
}

static void throw_bad_path2(void *ctx) {
  (void) ctx;
  scope {
    JD_VAR(x);
    JD_AV(path, 1); /* empty */
    jd_get_context(x, path, NULL, 1);
    jdt_diag("Exception not thrown, x=%lJ", x);
  }
}

static void test_exceptions(void) {
  jdt_throws(throw_unexpected, NULL,
             "Unexpected element in structure",
             "unexpected element in structure exception");
  jdt_throws(throw_bad_path, NULL,
             "Bad path",
             "bad path exception");
  jdt_throws(throw_bad_path2, NULL,
             "Bad path",
             "bad path exception");
}

static int nneq(void *a, void *b) {
  return a && a == b;
}

static void test_context(void) {
  scope {
    JD_JV(obj, "{\"one\":[1],\"two\":[2,4]}");
    JD_SV(ps, "$.one.0");
    JD_JV(pa, "[\"$\",\"one\",\"0\"]");
    JD_JV(pn, "[\"$\",\"one\",0]");

    ok(nneq(jd_get_context(obj, ps, NULL, 0), jd_get_context(obj, pa, NULL, 0)),
    "array path");
    ok(nneq(jd_get_context(obj, ps, NULL, 0), jd_get_context(obj, pn, NULL, 0)),
    "array path (numeric)");
  }

  scope {
    JD_VAR(obj);
    JD_JV(path, "[\"$\",0]");
    ok(!!jd_get_context(obj, path, NULL, 1), "vivify path");
    jdt_is_json(obj, "[null]", "created array");
  }
}

static void test_toker(void) {
  scope {
    JD_AV(want, 10);
    JD_AV(got, 10);
    jd_var *tok;
    jd__path_parser p;
    JD_SV(path, "$..foo[33]['a key']");

    jd_set_array_with(jd_push(want, 1), jd_niv('$'), NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv(JP_DOTDOT), NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv(JP_KEY), jd_nsv("foo"),  NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv('['), NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv(JP_INDEX), jd_niv(33),  NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv(']'), NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv('['), NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv(JP_KEY), jd_nsv("a key"),  NULL);
    jd_set_array_with(jd_push(want, 1), jd_niv(']'), NULL);

    jd__path_init_parser(&p, path);
    while (tok = jd__path_token(&p), tok)
      jd_assign(jd_push(got, 1), tok);

    jdt_is(got, want, "parse %V ok", path);
  }
}

static void test_parser(void) {
  scope {
    JD_3VARS(comp, cl, tmp);
    JD_SV(path, "$.foo.*.12");
    JD_JV(want, "[ [\"foo\"], [\"that\", \"this\"],  [12] ]");
    JD_JV(hash, "{\"this\":12,\"that\":true}");
    JD_AV(got, 10);
    comp = jd__path_compile(path);
    size_t cnt = jd_count(comp);
    for (unsigned i = 0; i < cnt; i++) {
      JD_AV(alt, 10);
      jd_eval(jd_get_idx(comp, i), cl, hash); /* closure returns closure... */
      for (;;) {
        jd_eval(cl, tmp, NULL); /* ...which returns literal */
        if (tmp->type == VOID) break;
        jd_assign(jd_push(alt, 1), tmp);
      }
      jd_assign(jd_push(got, 1), alt);
    }
    jdt_is(got, want, "parsed %V", path);
  }
}

void test_main(void) {
  test_toker();
  test_parser();
  test_exceptions();
  test_path();
  test_context();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
