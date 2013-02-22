/* map.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static int double_it(jd_var *rv, jd_var *ctx, jd_var *args) {
  jd_set_int(rv, jd_get_int(args) * 2);
  return 0;
}

static int is_odd(jd_var *rv, jd_var *ctx, jd_var *args) {
  jd_set_bool(rv, jd_get_int(args) & 1);
  return 0;
}

static int is_true(jd_var *rv, jd_var *ctx, jd_var *args) {
  jd_set_bool(rv, jd_test(args));
  return 0;
}

static void check_map(const char *json, const char *want, jd_closure_func f) {
  JD_BEGIN {
    JD_2VARS(in, out);
    JD_CV(cl, f);
    jd_from_jsonc(in, json);
    jd_map(out, cl, in);
    jdt_is_json(out, want, "map %s -> %s", json, want);
  }
  JD_END
}

static void check_grep(const char *json, const char *want, jd_closure_func f) {
  JD_BEGIN {
    JD_2VARS(in, out);
    JD_CV(cl, f);
    jd_from_jsonc(in, json);
    jd_grep(out, cl, in);
    jdt_is_json(out, want, "grep %s -> %s", json, want);
  }
  JD_END
}

static void check_dmap(const char *json, const char *want, jd_closure_func f) {
  JD_BEGIN {
    JD_2VARS(in, out);
    JD_CV(cl, f);
    jd_from_jsonc(in, json);
    jd_dmap(out, cl, in);
    jdt_is_json(out, want, "dmap %s -> %s", json, want);
  }
  JD_END
}

static void check_dgrep(const char *json, const char *want, jd_closure_func f) {
  JD_BEGIN {
    JD_2VARS(in, out);
    JD_CV(cl, f);
    jd_from_jsonc(in, json);
    jd_dgrep(out, cl, in);
    jdt_is_json(out, want, "dgrep %s -> %s", json, want);
  }
  JD_END
}

static void test_map(void) {
  check_map("1",
            "2", double_it);
  check_map("[1,2,3]",
            "[2,4,6]", double_it);
  check_map("[1,2,[3,4,5]]",
            "[2,4,6]", double_it);
  check_map("{\"one\":1,\"two\":2}",
            "{\"one\":2,\"two\":4}", double_it);
  check_map("[1,{\"one\":1,\"two\":2},3]",
            "[2,4,6]", double_it);
}

static void test_grep(void) {
  check_grep("1",
             "1", is_odd);
  check_grep("2",
             "null", is_odd);
  check_grep("[1,2,3]",
             "[1,3]", is_odd);
  check_grep("[1,2,[3,4,5]]",
             "[1,[3,4,5]]", is_odd);
  check_grep("{\"one\":1,\"two\":2}",
             "{\"one\":1}", is_odd);
  check_grep("[1,2,{\"one\":1,\"two\":2},3,4]",
             "[1,3]", is_odd);
  check_grep("[0,1,null,\"Hello\",[false,true,false,true,\"\"]]",
             "[1,\"Hello\",[false,true,false,true,\"\"]]", is_true);
}

static void test_dmap(void) {
  check_dmap("1",
             "2", double_it);
  check_dmap("[1,2,3]",
             "[2,4,6]", double_it);
  check_dmap("[1,2,[3,4,5]]",
             "[2,4,[6,8,10]]", double_it);
  check_dmap("{\"one\":1,\"two\":2}",
             "{\"one\":2,\"two\":4}", double_it);
  check_dmap("[1,{\"one\":1,\"two\":2},3]",
             "[2,{\"one\":2,\"two\":4},6]", double_it);
}

static void test_dgrep(void) {
  check_dgrep("1",
              "1", is_odd);
  check_dgrep("2",
              "null", is_odd);
  check_dgrep("[1,2,3]",
              "[1,3]", is_odd);
  check_dgrep("[1,2,[3,4,5]]",
              "[1,[3,5]]", is_odd);
  check_dgrep("{\"one\":1,\"two\":2}",
              "{\"one\":1}", is_odd);
  check_dgrep("[1,2,{\"one\":1,\"two\":2},3,4]",
              "[1,{\"one\":1},3]", is_odd);
  check_dgrep("[0,1,null,\"Hello\",[false,true,false,true,\"\"]]",
              "[1,\"Hello\",[true,true]]", is_true);

}

static void test_inplace(void) {
  JD_BEGIN {
    JD_VAR(x);
    JD_CV(dbl, double_it);
    JD_CV(odd, is_odd);
    jd_from_jsonc(x, "{\"foo\":[1,2,3]}");
    jd_dmap(x, dbl, x);
    jdt_is_json(x, "{\"foo\":[2,4,6]}", "inplace map");
    jd_dgrep(x, odd, x);
    jdt_is_json(x, "{\"foo\":[]}", "inplace grep");
  }
  JD_END
}

void test_main(void) {
  test_map();
  test_grep();
  test_dmap();
  test_dgrep();
  test_inplace();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
