/* model.t */

#include <stdio.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

const char *MODEL = "model.json";

static jd_var *load_string(jd_var *out, const char *filename) {
  FILE *f;
  char buf[512];
  size_t got;

  if (f = fopen(filename, "r"), !f) jd_die("Can't read %s", filename);
  jd_set_empty_string(out, 100);
  while (got = fread(buf, 1, sizeof(buf), f), got) {
    jd_append_bytes(out, buf, got);
  }
  fclose(f);
  return out;
}

static jd_var *load_json(jd_var *out, const char *filename) {
  jd_var json = JD_INIT;
  jd_from_json(out, load_string(&json, filename));
  jd_release(&json);
  return out;
}

void test_main(void) {
  jd_var json = JD_INIT;
  load_json(&json, MODEL);

  jdt_is_json(jd_rv(&json, "$.config.packagers.default.webroot"),
              "\"webroot/live/hls\"", "found value");

  jd_release(&json);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
