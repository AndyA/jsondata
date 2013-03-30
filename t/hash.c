/* hash.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static void test_hash(size_t sz) {
  jd_var ha = JD_INIT, keys = JD_INIT;
  jd_var k1 = JD_INIT, k2 = JD_INIT, k3 = JD_INIT;
  jd_var v1 = JD_INIT, v2 = JD_INIT, v3 = JD_INIT;
  int got;

  jd_set_string(&k1, "foo");
  jd_set_string(&k2, "bar");
  jd_set_string(&k3, "baz");

  jd_set_string(&v1, "value one");
  jd_set_string(&v2, "value two");
  jd_set_string(&v3, "value three");

  jd_set_hash(&ha, sz);

  jd_assign(jd_get_key(&ha, &k1, 1), &v1);
  jd_assign(jd_get_key(&ha, &k2, 1), &v2);

  is(jd_count(&ha), 2, "count = 2");

  ok(jd_compare(jd_get_key(&ha, &k1, 0), &v1) == 0, "found foo");
  null(jd_get_key(&ha, &k3, 0), "not found baz");

  jd_assign(jd_get_key(&ha, &k3, 1), &v3);

  jd_keys(&keys, &ha);
  /* TODO actually check the keys... */

  ok(jd_compare(jd_get_key(&ha, &k1, 0), &v1) == 0, "found foo again");
  ok(jd_compare(jd_get_key(&ha, &k3, 0), &v3) == 0, "found baz");

  got = jd_delete_key(&ha, &k1, NULL);
  is(got, 1, "deleted a foo");
  null(jd_get_key(&ha, &k1, 0), "not found foo");
  ok(jd_compare(jd_get_key(&ha, &k3, 0), &v3) == 0, "found baz again");
  ok(jd_compare(jd_get_key(&ha, &k2, 0), &v2) == 0, "bar's there too");

  jd_assign(jd_get_key(&ha, &k3, 1), &v1);
  ok(jd_compare(jd_get_key(&ha, &k3, 0), &v1) == 0, "found baz with new value");

  jd_release(&ha);
  jd_release(&keys);
  jd_release(&k1);
  jd_release(&k2);
  jd_release(&k3);
  jd_release(&v1);
  jd_release(&v2);
  jd_release(&v3);
}

static void test_ks(void) {
  jd_var h = JD_INIT;

  jd_set_hash(&h, 1);
  jd_set_string(jd_get_ks(&h, "name", 1), "foo");
  jd_set_int(jd_get_ks(&h, "value", 1), 1234);

  jdt_is_json(&h, "{\"name\":\"foo\",\"value\":1234}", "strings as keys");

  jd_set_real(jd_get_ks(&h, "value", 1), 1.25);

  jdt_is_json(&h, "{\"name\":\"foo\",\"value\":1.25}", "strings as keys, replace");

  jd_delete_ks(&h, "value", NULL);

  jdt_is_json(&h, "{\"name\":\"foo\"}", "strings as keys, delete");

  jd_release(&h);
}

static void test_hash_with(void) {
  scope {
    JD_VAR(ar);
    jd_set_hash_with(ar, jd_nsv("Hello"), jd_niv(1234), NULL);
    jdt_is_json(ar, "{\"Hello\":1234}", "jd_set_hash_with");
  }
}

static void throw_odd(void *ctx) {
  (void) ctx;
  scope {
    JD_VAR(x);
    jd_set_hash_with(x, jd_nsv("Hello"), jd_niv(9999), jd_nrv(1.24), NULL);
    jdt_diag("Exception not thrown, x=%lJ", x);
  }
}

static void test_exceptions(void) {
  jdt_throws(throw_odd, NULL,
             "Odd number of elements in hash initializer: 3",
             "Odd number of elements");
}

void test_main(void) {
  subtest("Bucket size 1") {
    test_hash(1);
  }

  subtest("Bucket size 10") {
    test_hash(10);
  }

  test_ks();
  test_hash_with();
  test_exceptions();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
