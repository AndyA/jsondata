/* t/get.c */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static void test_get(void) {
  scope {
    JD_JV(data, "{\"id\":[1,2,3]}");
    jdt_is_json(jd_get(data, jd_nsv("id"), 0), "[1,2,3]", "get hash");
    jd_assign(jd_get(data, jd_nsv("name"), 1), jd_nsv("Bogo"));
    jdt_is_json(jd_get(data, jd_nsv("name"), 0), "\"Bogo\"", "set hash");
  }
  scope {
    JD_JV(data, "[1,2,3]");
    jdt_is_json(jd_get(data, jd_niv(1), 0), "2", "get array");
  }
  scope {
    JD_JV(data, "[]");
    jd_assign(jd_get(data, jd_niv(0), 1), jd_nsv("First"));
    jd_assign(jd_get(data, jd_niv(1), 1), jd_nsv("Second"));
    jd_assign(jd_get(data, jd_niv(9), 1), jd_nsv("Tenth"));
    jdt_is_json(data,
    "[\"First\",\"Second\",null,null,null,null,null,null,null,\"Tenth\"]",
    "set array");
  }
}

void test_main(void) {
  test_get();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
