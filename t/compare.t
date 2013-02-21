/* compare.t */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jsondata.h"

static void check_sort(const char *in, const char *want,
                       int (*cmp)(jd_var *, jd_var *)) {
  JD_BEGIN {
    JD_SV(vin, in);
    JD_SV(vwant, want);
    JD_2VARS(jin, jwant);
    jd_from_json(jin, vin);
    jd_from_json(jwant, vwant);
    jd_sortv(jin, cmp);
    jdt_is(jin, jwant, "%s sorts as %s", in, want);
  }
  JD_CATCH(e) {
    jdt_diag("Exception: %V", e);
    fail("Exception: %s", jd_bytes(e, NULL));
  }
  JD_ENDCATCH
}

static int backwards(jd_var *a, jd_var *b) {
  return jd_compare(b, a);
}

static void test_compare(void) {
  JD_BEGIN {
    check_sort("[\"c\",\"b\",\"a\"]", "[\"a\",\"b\",\"c\"]", jd_compare);
    check_sort("[3,2,1]", "[1,2,3]", jd_compare);
    check_sort("[3.25,2.25,1.25]", "[1.25,2.25,3.25]", jd_compare);
    check_sort("[4,2,3.25,1.25,2,1.25]", "[1.25,1.25,2,2,3.25,4]", jd_compare);
    check_sort("[\"pi\",4,3.25,2,1.25]", "[1.25,2,3.25,4,\"pi\"]", jd_compare);
    check_sort("[true,false,\"pi\",4,3.25,2,1.25]",
    "[false,true,1.25,2,3.25,4,\"pi\"]", jd_compare);
    check_sort("[1,2,3]", "[3,2,1]", backwards);
  } JD_END
}

static void test_hashcode(void) {
  int i;
  static const char *obj[] = {
    "\"this is a long string\"",
    "\"This is a long string\"",
    "\"This is a long string.\"",
    "\"this is a long string.\"",
    "\"\"",
    "\" \"",
    "1", "100000", "100001", "2.0", "1.24",
    "\"1\"", "\"100000\"", "\"100001\"", "\"2.0\"", "\"1.24\"",
    "true", "\"true\"",
    "false", "\"false\"",
    "null", "\"null\"",
    NULL
  };
  JD_BEGIN {
    JD_HV(stats, 10);
    JD_3VARS(json, v, key);

    for (i = 0; obj[i]; i++) {
      jd_var *slot;

      jd_set_string(json, obj[i]);
      jd_from_json(v, json);
      jd_printf(key, "%lx", jd_hashcalc(v));

      slot = jd_get_key(stats, key, 1);
      jd_set_int(slot, jd_get_int(slot) + 1);
    }

    /*    jdt_diag("Hash stats: %lJ", stats);*/
    jd_keys(stats, key);
    for (i = 0; i < jd_count(key); i++) {
      jd_int count = jd_get_int(jd_get_key(stats, jd_get_idx(key, i), 0));
      ok(count < 3, "count for %d, %ld < 3", i, (long) count);
    }

  } JD_END
}

void test_main(void) {
  test_compare();
  test_hashcode();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
