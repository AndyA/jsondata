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

static void check_length(const char *src, size_t len) {
  scope {
    jd_var *str = jd_nsv(src);
    size_t got = jd_utf8_length(str);
    if (!ok(len == got, "length of %s is %lu", src, (unsigned long) len)) {
      diag("# got %lu", (unsigned long) got);
    }
    jd_var *chrs = jd_nav(got);
    for (unsigned pos = 0; pos != got; pos++) {
      jd_var *chr = jd_utf8_substr(jd_nv(), str, pos, 1);
      size_t chrlen = jd_utf8_length(chr);
      ok(chrlen == 1, "substr %u, length = 1 (got %lu)", pos, (unsigned long) chrlen);
      jd_assign(jd_push(chrs, 1), chr);
    }
    jd_var *joined = jd_join(jd_nv(), jd_nsv(""), chrs);
    jdt_is(joined, str, "%V reassembled as %V", str, joined);
  }
}

static void test_utf8_api(void) {
  check_length("", 0);
  check_length("\x31\x38\x30\xc2\xb0", 4);
  check_length("\x7f\xc2\x80\xdf\xbf\xe0\xa0\x80\xef\xbf\xbf\xf0\x90\x80\x80"
               "\xf7\xbf\xbf\xbf\xf8\x88\x80\x80\x80\xfb\xbf\xbf\xbf\xbf\xfc"
               "\x84\x80\x80\x80\x80\xfd\xbf\xbf\xbf\xbf\xbf", 11);
}

static void test_extract(void) {
  scope {
    uint32_t buf[ countof(u32) ];
    jd_var *in = jd_append_bytes(jd_nsv(""), u8, countof(u8));

    size_t len = jd_utf8_length(in);
    ok(len == countof(u32), "utf8 length is %lu (got %lu)",
    (unsigned long) countof(u32), (unsigned long) len);

    size_t got = jd_utf8_extract(buf, in, 0, len);
    ok(got == len, "extracted length is %lu (got %lu)",
    (unsigned long) len, (unsigned long) got);

    ok(0 == memcmp(u32, buf, sizeof(u32)), "extracted buffer matches");

    for (unsigned i = 0; i < len; i++) {
      uint32_t ch;
      jd_utf8_extract(&ch, in, i, 1);
      ok(ch == u32[i], "individual char %u (%08x == %08x)", i, ch, u32[i]);
    }
  }
}

static void test_append(void) {
  scope {
    jd_var *str = jd_utf8_append(jd_nsv(""), u32, countof(u32));
    size_t ulen = jd_utf8_length(str);
    ok(ulen == countof(u32), "utf8 length is %lu (got %lu)",
    (unsigned long) countof(u32), (unsigned long) ulen);
    size_t len = jd_length(str);
    ok(len == countof(u8), "raw length is %lu (got %lu)",
    (unsigned long) countof(u8), (unsigned long) len);
    ok(0 == memcmp(jd_bytes(str, NULL), u8, countof(u8)), "buffer OK");
  }
}

static void test_un_pack(void) {
  scope {
    jd_var *str = jd_utf8_append(jd_nsv(""), u32, countof(u32));
    jd_var *chr = jd_utf8_unpack(jd_nv(), str);
    if (ok(jd_count(chr) == countof(u32), "unpack length")) {
      for (unsigned i = 0; i < countof(u32); i++) {
        ok(jd_get_int(jd_get_idx(chr, i)) == (jd_int) u32[i], "char[%u] = %08lx",
        i, (unsigned long) u32[i]);
      }
    }

    jd_var *pack1 = jd_utf8_pack(jd_nv(), chr);
    jdt_is(pack1, str, "pack short string");

    while (jd_count(chr) < 1000) {
      jd_append(str, str);
      jd_append(chr, chr);
    }

    jd_var *pack2 = jd_utf8_pack(jd_nv(), chr);
    jdt_is(pack2, str, "pack long string");

    jd_var *chr2 = jd_utf8_unpack(jd_nv(), str);
    jdt_is(chr, chr2, "unpack long string");
  }
}

void test_main(void) {
  scope {} /* memory accounting */
  test_basic();
  test_span();
  test_utf8_api();
  test_extract();
  test_append();
  test_un_pack();
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
