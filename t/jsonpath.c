/* path.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"
#include "jd_path.h"

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
    jd_set_array_with(jd_push(want, 1), jd_niv(JP_SLICE), jd_niv(33),  NULL);
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

static void test_compile(void) {
  scope {
    JD_SV(path1, "$.foo.*.12");
    JD_VAR(path2);

    jd_var *c1 = jd__path_compile(path1);
    not_null(c1, "compiled path not null");
    jd_assign(path2, path1);
    jd_var *c2 = jd__path_compile(path2);
    ok(c1 == c2, "string assign keeps magic");

    jd_set_string(path1, "$.bar");
    jd_var *c3 = jd__path_compile(path1);
    not_null(c1, "compiled path not null");
    ok(c3 != c1, "change string: new magic");
  }
}

static void test_parser(void) {
  scope {
    JD_3VARS(comp, cl, tmp);
    JD_SV(path, "$.foo.*.12");
    JD_JV(want, "[ [\"$\"], [\"foo\"], [\"that\", \"this\"], [12] ]");
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

static jd_var *array_with_args(jd_var *out, va_list ap) {
  jd_set_array(out, 10);
  const char *v;

  while (v = va_arg(ap, const char *), v) {
    jd_set_string(jd_push(out, 1), v);
  }

  return out;
}

static void test_traverse(void) {
  scope {
    JD_JV(data, "{\"id\":[1,2,3]}");
    jdt_is_json(jd__traverse_path(data, jd_nsv("id"), 0), "[1,2,3]", "traverse hash");
  }
  scope {
    JD_JV(data, "[1,2,3]");
    jdt_is_json(jd__traverse_path(data, jd_niv(1), 0), "2", "traverse array");
  }
  scope {
    JD_VAR(data);
    jd_assign(jd__traverse_path(data, jd_nsv("id"), 1), jd_nsv("Bongo"));
    jdt_is_json(data, "{\"id\":\"Bongo\"}", "vivify hash");
  }
  scope {
    JD_VAR(data);
    jd_assign(jd__traverse_path(data, jd_niv(3), 1), jd_nsv("Bongo"));
    jdt_is_json(data, "[null,null,null,\"Bongo\"]", "vivify array");
  }
  scope {
    JD_VAR(data);
    jd_assign(jd__traverse_path(data, jd_nsv("1"), 1), jd_nsv("Bongo"));
    jdt_is_json(data, "[null,\"Bongo\"]", "vivify array, string key");
  }
  scope {
    JD_JV(data, "{\"id\":[1,2,3]}");
    JD_JV(path, "[\"id\",1]");
    jdt_is_json(jd__traverse_path(data, path, 0), "2", "traverse hash path");
  }
  scope {
    JD_JV(data, "{\"id\":[1,2,3]}");
    JD_JV(path, "[\"id\",\"1\"]");
    jdt_is_json(jd__traverse_path(data, path, 0), "2", "traverse hash path, string key");
  }
  scope {
    JD_VAR(data);
    JD_JV(path, "[\"id\",1]");
    jd_assign(jd__traverse_path(data, path, 1), jd_nsv("Boo!"));
    jdt_is_json(data, "{\"id\":[null,\"Boo!\"]}", "vivify path");
  }
  scope {
    JD_VAR(data);
    JD_JV(path, "[\"id\",\"1\"]");
    jd_assign(jd__traverse_path(data, path, 1), jd_nsv("Boo!"));
    jdt_is_json(data, "{\"id\":[null,\"Boo!\"]}", "vivify path, string key");
  }
}

static jd_var *check_iter(const char *json, const char *path, int vivify,
                          const char *expect, ...) {
  JD_SV(pathv, path);
  JD_AV(got, 10);
  JD_JV(v, json);
  JD_3VARS(iter, cpath, want);
  jd_var *slot;

  va_list ap;
  va_start(ap, expect);
  array_with_args(want, ap);
  va_end(ap);

  jd_path_iter(iter, v, pathv, vivify);
  while (slot = jd_path_next(iter, cpath, NULL), slot) {
    jd_assign(jd_push(got, 1), cpath);
    jd_assign(slot, cpath);
  }
  jdt_is(got, want, "iterated %s", path);
  jdt_is_json(v, expect, "data structure vivified");
  return v;
}


static void test_iter(void) {
  scope {
    check_iter("{}", "$.foo", 1,
    "{\"foo\":\"$.foo\"}",
    "$.foo", NULL);

    check_iter("{}", "$.foo[bar,baz]", 1,
    "{\"foo\":{\"bar\":\"$.foo.bar\",\"baz\":\"$.foo.baz\"}}",
    "$.foo.bar", "$.foo.baz", NULL);

    check_iter("{}", "$[bar,baz].foo", 1,
    "{\"bar\":{\"foo\":\"$.bar.foo\"},\"baz\":{\"foo\":\"$.baz.foo\"}}",
    "$.bar.foo", "$.baz.foo", NULL);

    check_iter("{}", "$.foo[0:3]", 1,
    "{\"foo\":[\"$.foo.0\",\"$.foo.1\",\"$.foo.2\"]}",
    "$.foo.0",
    "$.foo.1",
    "$.foo.2",
    NULL);

    check_iter("{}", "$.foo[0:3]['bar','baz']", 1,
    "{\"foo\":[{\"bar\":\"$.foo.0.bar\",\"baz\":\"$.foo.0.baz\"},"
    "{\"bar\":\"$.foo.1.bar\",\"baz\":\"$.foo.1.baz\"},"
    "{\"bar\":\"$.foo.2.bar\",\"baz\":\"$.foo.2.baz\"}]}",
    "$.foo.0.bar",
    "$.foo.0.baz",
    "$.foo.1.bar",
    "$.foo.1.baz",
    "$.foo.2.bar",
    "$.foo.2.baz",
    NULL);

    check_iter("{}", "$.foo[0:3][0:10:5]", 1,
    "{\"foo\":[[\"$.foo.0.0\",null,null,null,null,\"$.foo.0.5\"],"
    "[\"$.foo.1.0\",null,null,null,null,\"$.foo.1.5\"],"
    "[\"$.foo.2.0\",null,null,null,null,\"$.foo.2.5\"]]}",
    "$.foo.0.0",
    "$.foo.0.5",
    "$.foo.1.0",
    "$.foo.1.5",
    "$.foo.2.0",
    "$.foo.2.5",
    NULL);
  }
}

void test_main(void) {
  test_toker();
  test_parser();
  test_compile();
  test_traverse();
  test_iter();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
