/* hash.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jsondata.h"

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

  jd_keys(&ha, &keys);
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

void test_main(void) {
  nest_in("Bucket size 1");
  test_hash(1);
  nest_out();
  nest_in("Bucket size 10");
  test_hash(10);
  nest_out();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
