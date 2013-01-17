/* jd_json.c */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsondata.h"

#define NEED_ESCAPE(c) \
  ((c) < ' ' || (c) == 0x7f || (c) == '"' || (c) == '\\')

static jd_var *escape_string(jd_var *out, jd_var *str) {
  size_t sz;
  char tmp[8];
  const char *buf = jd_bytes(str, &sz);
  const char *be = buf + sz - 1;
  const char *bp, *ep;

  jd_set_empty_string(out, sz);

  for (bp = buf; bp != be; bp++) {
    if (NEED_ESCAPE(*bp)) {
      jd_append_bytes(out, buf, bp - buf);
      switch (*bp) {
      case 0x08:
        ep = "\\b";
        break;
      case 0x09:
        ep = "\\t";
        break;
      case 0x0A:
        ep = "\\n";
        break;
      case 0x0C:
        ep = "\\f";
        break;
      case 0x0D:
        ep = "\\r";
        break;
      case '"':
        ep = "\\\"";
        break;
      case '\\':
        ep = "\\\\";
        break;
      default:
        sprintf(tmp, "\\u%04X", (unsigned char) *bp);
        ep = tmp;
        break;
      }
      jd_append_bytes(out, ep, strlen(ep));
      buf = bp + 1;
    }
  }
  jd_append_bytes(out, buf, bp - buf);
  return out;
}

static void to_json_string(jd_var *out, jd_var *str) {
  jd_set_string(jd_push(out, 1), "\"");
  escape_string(jd_push(out, 1), str);
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

static void to_json_hash(jd_var *out, jd_var *ha) {
  jd_var tmp = JD_INIT, sep = JD_INIT;
  jd_var keys = JD_INIT;
  size_t count;
  unsigned i;

  jd_keys(ha, &keys);
  jd_sort(&keys);
  count = jd_count(&keys);
  jd_set_string(&sep, "");
  jd_set_array(&tmp, count * 4);

  for (i = 0; i < count; i++) {
    jd_var *k = jd_get_idx(&keys, i);
    if (i) jd_set_string(jd_push(&tmp, 1), ",");
    jd_to_json(jd_push(&tmp, 1), k);
    jd_set_string(jd_push(&tmp, 1), ":");
    jd_to_json(jd_push(&tmp, 1), jd_get_key(ha, k, 0));
  }

  jd_set_string(jd_push(out, 1), "{");
  jd_join(jd_push(out, 1), &sep, &tmp);
  jd_set_string(jd_push(out, 1), "}");

  jd_release(&sep);
  jd_release(&tmp);
  jd_release(&keys);
}

static void to_json(jd_var *out, jd_var *v) {
  switch (v->type) {
  case STRING:
    to_json_string(out, v);
    break;
  case ARRAY:
    to_json_array(out, v);
    break;
  case HASH:
    to_json_hash(out, v);
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

struct parser {
  const char *pp;
  const char *ep;
};

#define CHAR(p) (*((p)->pp))
#define JUMP(p, n) do { (p)->pp += (n); } while (0)
#define STEP(p) JUMP(p, 1)
#define SKIP(p) do { while (isspace(CHAR(p))) STEP(p); } while (0)

static size_t p_has(struct parser *p, const char *s) {
  size_t l = strlen(s);
  if (p->pp + l <= p->ep && 0 == memcmp(p->pp, s, l))
    return l;
  return 0;
}

static jd_var *from_json(jd_var *out, struct parser *p);

static jd_var *from_json_array(jd_var *out, struct parser *p) {
  jd_set_array(out, 10);
  for (;;) {
    STEP(p);
    if (CHAR(p) == ']') break;
    from_json(jd_push(out, 1), p);
    SKIP(p);
    if (CHAR(p) == ']') break;
    if (CHAR(p) != ',') jd_die("Expected comma or closing bracket");
  }
  STEP(p);
  return out;
}

static jd_var *from_json_hash(jd_var *out, struct parser *p) {
  jd_var key = JD_INIT;
  jd_set_hash(out, 10);
  for (;;) {
    STEP(p);
    if (CHAR(p) == '}') break;
    from_json(&key, p);
    SKIP(p);
    if (CHAR(p) != ':') jd_die("Missing colon");
    STEP(p);
    from_json(jd_get_key(out, &key, 1), p);
    SKIP(p);
    if (CHAR(p) == '}') break;
    if (CHAR(p) != ',') jd_die("Expected comma or closing brace");
  }
  STEP(p);
  jd_release(&key);
  return out;
}

static void append_byte(jd_var *out, char c) {
  jd_append_bytes(out, &c, 1);
}

static unsigned parse_escape(struct parser *p) {
  char buf[5], *ep;
  unsigned esc;
  if (p->pp + 4 > p->ep) goto bad;
  memcpy(buf, p->pp, 4);
  buf[4] = '\0';
  esc = (unsigned) strtoul(buf, &ep, 16);
  if (*ep) goto bad;
  JUMP(p, 3);
  return esc;

bad:
  jd_die("Bad escape");
  return 0;
}

static jd_var *from_json_string(jd_var *out, struct parser *p) {
  jd_set_empty_string(out, 16);
  char c;
  for (;;) {
    STEP(p);
    c = CHAR(p);
    if (c == '"') break;
    if (c == '\\') {
      STEP(p);
      switch (CHAR(p)) {
      case 'b':
        append_byte(out, 0x08);
        break;
      case 't':
        append_byte(out, 0x09);
        break;
      case 'n':
        append_byte(out, 0x0A);
        break;
      case 'f':
        append_byte(out, 0x0C);
        break;
      case 'r':
        append_byte(out, 0x0D);
        break;
      case '\\':
        append_byte(out, '\\');
        break;
      case '"':
        append_byte(out, '"');
        break;
      case '/':
        append_byte(out, '/');
        break;
      case 'u':
        STEP(p);
        append_byte(out, parse_escape(p));
        break;
      }
    }
    else {
      append_byte(out, c);
    }
  }
  STEP(p);
  return out;
}

static jd_var *from_json_bool(jd_var *out, struct parser *p) {
  size_t sz;
  if (sz = p_has(p, "true"), sz) {
    JUMP(p, sz);
    return jd_set_bool(out, 1);
  }
  if (sz = p_has(p, "false"), sz) {
    JUMP(p, sz);
    return jd_set_bool(out, 0);
  }
  jd_die("Expected true or false");
  return NULL;
}

static jd_var *from_json_null(jd_var *out, struct parser *p) {
  size_t sz;
  if (sz = p_has(p, "null"), sz) {
    JUMP(p, sz);
    return jd_set_void(out);
  }
  jd_die("Expected null");
  return NULL;
}

static jd_var *from_json_num(jd_var *out, struct parser *p) {
  char *endp;
  jd_int i;
  double r;

  i = strtoll(p->pp, &endp, 10);
  if (endp != p->pp && !isalnum(*endp) && *endp != '.') {
    p->pp = endp;
    return jd_set_int(out, i);
  }

  r = strtod(p->pp, &endp);
  if (endp != p->pp) {
    p->pp = endp;
    return jd_set_real(out, r);
  }

  jd_die("Syntax error");
  return NULL;
}

static jd_var *from_json(jd_var *out, struct parser *p) {
  SKIP(p);
  switch (CHAR(p)) {
  case '[':
    return from_json_array(out, p);
  case '{':
    return from_json_hash(out, p);
  case '"':
    return from_json_string(out, p);
  case 't':
  case 'f':
    return from_json_bool(out, p);
  case 'n':
    return from_json_null(out, p);
  default:
    return from_json_num(out, p);
  }

  return out;
}

jd_var *jd_from_json(jd_var *out, jd_var *json) {
  jd_var tmp = JD_INIT;
  struct parser p;
  size_t size;

  jd_stringify(&tmp, json);
  p.pp = jd_bytes(&tmp, &size);
  p.ep = p.pp + size - 1;
  from_json(out, &p);
  jd_release(&tmp);

  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */