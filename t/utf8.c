/* t/utf8.c */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"
#include "jd_utf8.h"

static uint32_t u32[] = {
  0x00000000, 0x0000007F, 0x00000080, 0x000007FF, 0x00000800, 0x0000FFFF,
  0x00010000, 0x001FFFFF, 0x00200000, 0x03FFFFFF, 0x04000000, 0x7FFFFFFF
};

static uint8_t u8[] = {
  0x00, 0x7f, 0xc2, 0x80, 0xdf, 0xbf, 0xe0, 0xa0, 0x80, 0xef, 0xbf, 0xbf,
  0xf0, 0x90, 0x80, 0x80, 0xf7, 0xbf, 0xbf, 0xbf, 0xf8, 0x88, 0x80, 0x80,
  0x80, 0xfb, 0xbf, 0xbf, 0xbf, 0xbf, 0xfc, 0x84, 0x80, 0x80, 0x80, 0x80,
  0xfd, 0xbf, 0xbf, 0xbf, 0xbf, 0xbf
};

#define countof(ar) (sizeof(ar) / sizeof(ar[0]))

static void hd8(const char *name, uint8_t *pp, size_t len) {
  unsigned pos = 0, i;
  while (len) {
    fprintf(stderr, "# %s %08x :", name, pos);
    for (i = 0; i < 16 && i < len; i++)
      fprintf(stderr, " %02x", *pp++);
    fprintf(stderr, "\n");
    len -= i;
    pos += i;
  }
}

static void hd32(const char *name, uint32_t *pp, size_t len) {
  unsigned pos = 0, i;
  while (len) {
    fprintf(stderr, "# %s %08x :", name, pos);
    for (i = 0; i < 4 && i < len; i++)
      fprintf(stderr, " %08x", *pp++);
    fprintf(stderr, "\n");
    len -= i;
    pos += i;
  }
}

static void test_basic(void) {
  uint32_t buf32[countof(u32)];
  uint8_t buf8[countof(u8)];

  struct buf8 b8;
  struct buf32 b32;

  subtest("utf32 -> utf8") {

    b32.pos = u32;
    b32.lim = u32 + countof(u32);

    b8.pos = buf8;
    b8.lim = buf8 + countof(u8);

    jd__to_utf8(&b8, &b32);

    if (!ok(b8.pos == b8.lim, "buffer filled"))
      diag("buf = %p, pos = %p, lim = %p", buf8, b8.pos, b8.lim);
    if (!ok(0 == memcmp(u8, buf8, sizeof(u8)), "buffer matches")) {
      hd8("want", u8, countof(u8));
      hd8("got ", buf8, b8.pos - buf8);
    }
  }

  subtest("utf8 -> utf32") {
    /* utf8 -> utf32 */

    b8.pos = u8;
    b8.lim = u8 + countof(u8);

    b32.pos = buf32;
    b32.lim = buf32 + countof(u32);

    jd__from_utf8(&b32, &b8);

    if (!ok(b32.pos == b32.lim, "buffer filled"))
      diag("buf = %p, pos = %p, lim = %p", buf32, b32.pos, b32.lim);
    if (!ok(0 == memcmp(u32, buf32, sizeof(u32)), "buffer matches")) {
      hd32("want", u32, countof(u32));
      hd32("got ", buf32, b32.pos - buf32);
    }
  }
}

static void test_span(void) {
  size_t rem;
  ok(jd__span_utf32(u32, countof(u32)) == countof(u8), "utf32 length");
  ok(jd__span_utf8(u8, countof(u8), &rem) == countof(u32), "utf8 length");
  ok(rem == 0, "utf8 no remainder");
}

void test_main(void) {
  scope {} /* memory accounting */
  test_basic();
  test_span();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
