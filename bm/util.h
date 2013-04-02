/* util.h */

#ifndef __UTIL_H
#define __UTIL_H

#include "jsondata.h"

extern const char *test_name;

void setup(void);
void test(void);
void teardown(void);

void count_ops(unsigned long n);
jd_var *load_string(jd_var *out, const char *filename);
jd_var *load_json(jd_var *out, const char *filename);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
