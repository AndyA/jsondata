/* jdtest.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "jd_pretty.h"

static jd_var *load_string(jd_var *out, const char *filename) {
  FILE *f;
  char buf[512];
  size_t got;

  if (f = fopen(filename, "r"), !f) jd_die("Can't read %s: %m", filename);
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

#if 0
static jd_var *save_json(jd_var *data, const char *filename) {
  scope {
    FILE *f;
    if (f = fopen(filename, "w"), !f) jd_die("Can't write %s: %m", filename);
    jd_fprintf(f, "%V", jd_to_json_pretty(jd_nv(), data));
    fclose(f);
  }
  return data;
}
#endif

int main(int argc, char *argv[]) {
  scope {
    jd_var *path = jd_nsv("$.*.*.annotation");
    for (int a = 1; a < argc; a++) {
      scope {
        jd_var *data = load_json(jd_nv(), argv[a]);
        jd_var *iter = jd_path_iter(jd_nv(), data, path, 0);
        jd_var *cpath = jd_nv();
        jd_var *slot;
        while (slot = jd_path_next(iter, cpath, NULL), slot) {
          jd_printf("%V: %J\n", cpath, slot);
        }
      }
    }
  }
  return 0;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
