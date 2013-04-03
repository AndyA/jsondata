/* jd_path.h */

#ifndef JD_PATH_H_
#define JD_PATH_H_

#include "jsondata.h"

typedef struct {
  jd_var *path; /* source string */
  const char *sp, *cp, *ep;
} jd__path_parser;

enum {
  JP_DOTDOT,
  JP_SLICE,
  JP_KEY
};

jd_var *jd__traverse_path(jd_var *v, jd_var *path, int vivify);
void jd__path_init_parser(jd__path_parser *p, jd_var *path);
jd_var *jd__path_token(jd__path_parser *p);
jd_var *jd__path_parse(jd_var *out, jd_var *path);
jd_var *jd__path_compile(jd_var *path);
jd_var *jd__path_to_string(jd_var *volatile out, jd_var *path);

#endif


/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
