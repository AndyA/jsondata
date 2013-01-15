/* tap.h */

#ifndef __TAP_H
#define __TAP_H

void diag(const char *fmt, ...);
void die(const char *fmt, ...);
void done_testing(void);
void nest_in(const char *p);
void nest_out(void);
int ok(int flag, const char *msg, ...);
int pass(const char *msg, ...);
int fail(const char *msg, ...);
int is(long long got, long long want, const char *msg, ...);
int not_null(const void *p, const char *msg, ...);
int null(const void *p, const char *msg, ...);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
