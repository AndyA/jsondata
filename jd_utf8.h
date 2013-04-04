/* jd_utf8.h */

#ifndef JD_UTF8_H_
#define JD_UTF8_H_

#include <stdint.h>

struct buf8 {
  uint8_t *pos;
  uint8_t *lim;
};

struct buf32 {
  uint32_t *pos;
  uint32_t *lim;
};

void jd__to_utf8(struct buf8 *out, struct buf32 *in);
void jd__from_utf8(struct buf32 *out, struct buf8 *in);
size_t jd__span_utf8(uint8_t *buf, size_t len, size_t *rem);
size_t jd__span_utf32(uint32_t *buf, size_t len);

#endif

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
