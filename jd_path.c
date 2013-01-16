/* jd_path.c */

#include "jsondata.h"

jd_var *jd_get_context(jd_var *root, jd_var *path, jd_context *ctx, int vivify) {
  return NULL;
}

static jd_var *getter(jd_var *root, const char *path, int vivify) {
  jd_var *rv, pv = JD_INIT;
  jd_set_string(&pv, path);
  rv = jd_get_context(root, &pv, NULL, vivify);
  jd_release(&pv);
  return rv;
}

jd_var *jd_lv(jd_var *root, const char *path) {
  return getter(root, path, 1);
}

jd_var *jd_rv(jd_var *root, const char *path) {
  return getter(root, path, 0);
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
