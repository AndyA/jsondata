/* clone.t */

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void test_clone(void) {
  jd_var dst = JD_INIT, src = JD_INIT, ref = JD_INIT;
  jd_var json = JD_INIT;

  jd_set_string(&json, "{\"a\":[1,2,3],\"b\":3.14,\"c\":\"foo\"}");
  jd_from_json(&ref, &json);

  jd_clone(&src, &ref, 1);
  jd_clone(&dst, &src, 0); /* shallow */
  jdt_is(&dst, &src, "clone matches");
  jd_assign(jd_lv(&dst, "$.d.0.capsule.source"), &json);
  jdt_is(&ref, &src, "source unchanged");
  jd_assign(jd_lv(&dst, "$.a.3"), &json);
  jdt_is(jd_rv(&src, "$.a.3"), &json, "deep item changed");

  jd_clone(&src, &ref, 1);
  jd_clone(&dst, &src, 1); /* deep */
  jdt_is(&dst, &src, "clone matches");
  jd_assign(jd_lv(&dst, "$.d.0.capsule.source"), &json);
  jdt_is(&ref, &src, "source unchanged");
  jd_assign(jd_lv(&dst, "$.a.3"), &json);
  jdt_is(&ref, &src, "source unchanged again");

  jd_release(&dst);
  jd_release(&src);
  jd_release(&ref);
  jd_release(&json);
}

static void test_merge(void) {
  jd_var dst = JD_INIT, src = JD_INIT, ref = JD_INIT;

  jd_from_jsons(&dst, "{\"a\":[1,2,3],\"b\":3.14,\"c\":\"foo\"}");
  jd_from_jsons(&src, "{\"c\":[4,5,6],\"d\":3.14,\"e\":\"foo\"}");

  jd_merge(&dst, &src, 1);
  jdt_is_json(&dst,
              "{\"a\":[1,2,3],\"b\":3.14,\"c\":[4,5,6],\"d\":3.14,\"e\":\"foo\"}",
              "merged");

  jd_release(&dst);
  jd_release(&src);
  jd_release(&ref);
}

void test_main(void) {
  test_clone();
  test_merge();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
