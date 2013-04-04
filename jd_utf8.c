/* jd_utf8.c */

#include <stdio.h>

#include "jd_utf8.h"
#include "jd_pretty.h"

/*
 *    7 00000000 0000007F 1 0xxxxxxx
 *   11 00000080 000007FF 2 110xxxxx  10xxxxxx
 *   16 00000800 0000FFFF 3 1110xxxx  10xxxxxx  10xxxxxx
 *   21 00010000 001FFFFF 4 11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
 *   26 00200000 03FFFFFF 5 111110xx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
 *   31 04000000 7FFFFFFF 6 1111110x  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
 */

#define PUT(buf, ch)  do { *(buf->pos)++ = (ch); } while (0)
#define NEED(buf, sz) do { if (buf->pos + (sz) > buf->lim) break; } while (0)
#define TAKE(buf, sz) do { NEED(buf, sz); buf->pos += (sz); } while (0)

void jd__to_utf8(struct buf8 *out, struct buf32 *in) {
  while (in->pos < in->lim) {
    uint32_t ch = *(in->pos);

    if (ch >= 0x00000080) {
      if (ch >= 0x00000800) {
        if (ch >= 0x00010000) {
          if (ch >= 0x00200000) {
            if (ch >= 0x04000000) {
              NEED(out, 6);
              PUT(out, 0xfc | ((ch & 0x40000000) >> 30));
              PUT(out, 0x80 | ((ch & 0x3f000000) >> 24));
            }
            else {
              NEED(out, 5);
              PUT(out, 0xf8 | ((ch & 0x03000000) >> 24));
            }
            PUT(out, 0x80 | ((ch & 0x00fc0000) >> 18));
          }
          else {
            NEED(out, 4);
            PUT(out, 0xf0 | ((ch & 0x001c0000) >> 18));
          }
          PUT(out, 0x80 | ((ch & 0x0003f000) >> 12));
        }
        else {
          NEED(out, 3);
          PUT(out, 0xe0 | ((ch & 0x0000f000) >> 12));
        }
        PUT(out, 0x80 | ((ch & 0x00000fc0) >>  6));
      }
      else {
        NEED(out, 2);
        PUT(out, 0xc0 | ((ch & 0x000007c0) >>  6));
      }
      PUT(out, 0x80 | ((ch & 0x0000003f) >>  0));
    }
    else {
      NEED(out, 1);
      PUT(out, ch);
    }

    in->pos++;
  }
}

void jd__from_utf8(struct buf32 *out, struct buf8 *in) {
  while (in->pos < in->lim && out->pos < out->lim) {
    uint8_t *cp = in->pos;
    uint32_t och;
    uint8_t ch = cp[0];
    if ((ch & 0xfe) == 0xfc) {
      TAKE(in, 6);
      och = (uint32_t)(ch & 0x01) << 30 |
            (uint32_t)(cp[1] & 0x3f) << 24 |
            (uint32_t)(cp[2] & 0x3f) << 18 |
            (uint32_t)(cp[3] & 0x3f) << 12 |
            (uint32_t)(cp[4] & 0x3f) <<  6 |
            (uint32_t)(cp[5] & 0x3f) <<  0;
    }
    else if ((ch & 0xfc) == 0xf8) {
      TAKE(in, 5);
      och = (uint32_t)(ch & 0x03) << 24 |
            (uint32_t)(cp[1] & 0x3f) << 18 |
            (uint32_t)(cp[2] & 0x3f) << 12 |
            (uint32_t)(cp[3] & 0x3f) <<  6 |
            (uint32_t)(cp[4] & 0x3f) <<  0;
    }
    else if ((ch & 0xf8) == 0xf0) {
      TAKE(in, 4);
      och = (uint32_t)(ch & 0x07) << 18 |
            (uint32_t)(cp[1] & 0x3f) << 12 |
            (uint32_t)(cp[2] & 0x3f) <<  6 |
            (uint32_t)(cp[3] & 0x3f) <<  0;
    }
    else if ((ch & 0xf0) == 0xe0) {
      TAKE(in, 3);
      och = (uint32_t)(ch & 0x0f) << 12 |
            (uint32_t)(cp[1] & 0x3f) <<  6 |
            (uint32_t)(cp[2] & 0x3f) <<  0;
    }
    else if ((ch & 0xe0) == 0xc0) {
      TAKE(in, 2);
      och = (uint32_t)(ch & 0x1f) << 6 |
            (uint32_t)(cp[1] & 0x3f) <<  0;
    }
    else {
      TAKE(in, 1);
      och = ch;
    }
    *(out->pos)++ = och;
  }
}

size_t jd__span_utf8(uint8_t *buf, size_t len, size_t *rem) {
  size_t len32 = 0;
  uint8_t *lim = buf + len;
  while (buf != lim) {
    uint8_t ch = *buf;
    size_t step = ((ch & 0xfe) == 0xfc) ? 6
                  : ((ch & 0xfc) == 0xf8) ? 5
                  : ((ch & 0xf8) == 0xf0) ? 4
                  : ((ch & 0xf0) == 0xe0) ? 3
                  : ((ch & 0xe0) == 0xc0) ? 2 : 1;
    if (buf + step > lim) break;
    len32++;
    buf += step;
  }
  if (rem) *rem = buf - lim;
  return len32;
}

size_t jd__span_utf32(uint32_t *buf, size_t len) {
  size_t len8 = 0;
  uint32_t *lim = buf + len;

  while (buf != lim) {
    uint32_t ch = *buf++;
    len8 += ch >= 0x04000000 ? 6
            : ch >= 0x00200000 ? 5
            : ch >= 0x00010000 ? 4
            : ch >= 0x00000800 ? 3
            : ch >= 0x00000080 ? 2
            : 1;
  }

  return len8;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
