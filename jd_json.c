/* jd_json.c */

#include "jsondata.h"

static void to_json_string(jd_var *out, jd_var *str) {
  jd_set_string(jd_push(out, 1), "\"");
  jd_assign(jd_push(out, 1), str);
  jd_set_string(jd_push(out, 1), "\"");
}

static void to_json_array(jd_var *out, jd_var *ar) {
  jd_var tmp = JD_INIT, sep = JD_INIT;
  unsigned i;
  size_t count = jd_count(ar);

  jd_set_string(&sep, ",");
  jd_set_array(&tmp, count);

  for (i = 0; i < count; i++) {
    jd_to_json(jd_push(&tmp, 1), jd_get_idx(ar, i));
  }

  jd_set_string(jd_push(out, 1), "[");
  jd_join(jd_push(out, 1), &sep, &tmp);
  jd_set_string(jd_push(out, 1), "]");

  jd_release(&sep);
  jd_release(&tmp);
}

static void to_json(jd_var *out, jd_var *v) {
  switch (v->type) {
  case STRING:
    to_json_string(out, v);
    break;
  case ARRAY:
    to_json_array(out, v);
    break;
  default:
    jd_stringify(jd_push(out, 1), v);
    break;
  }
}

jd_var *jd_to_json(jd_var *out, jd_var *v) {
  jd_var tmp = JD_INIT, sep = JD_INIT;
  jd_set_array(&tmp, 1);
  jd_set_string(&sep, "");
  to_json(&tmp, v);
  jd_join(out, &sep, &tmp);
  jd_release(&sep);
  jd_release(&tmp);
  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
