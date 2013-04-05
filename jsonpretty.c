/* jdtest.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "jd_pretty.h"

static jd_var *load_string(jd_var *out, FILE *f) {
  char buf[512];
  size_t got;

  jd_set_empty_string(out, 10000);
  while (got = fread(buf, 1, sizeof(buf), f), got)
    jd_append_bytes(out, buf, got);
  return out;
}

static jd_var *load_json(jd_var *out, FILE *f) {
  jd_var json = JD_INIT;
  jd_from_json(out, load_string(&json, f));
  jd_release(&json);
  return out;
}

int main(void) {
  scope {
    jd_fprintf(stdout, "%lJ", load_json(jd_nv(), stdin));
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
