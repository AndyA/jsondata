/* jd_printf.c */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "jd_private.h"
#include "jsondata.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static jd_string *safe_vprintf(jd_string *jds, const char *fmt, va_list ap) {
  jd__string_empty(jds);
  for (;;) {
    va_list aq;
    va_copy(aq, ap);
    size_t sz = vsnprintf(jds->data, jds->size, fmt, aq);
    va_end(aq);
    if (sz < jds->size) {
      jds->used = sz + 1;
      return jds;
      break;
    }
    jd__string_ensure(jds, sz + 1);
  }
}

static char *find_sub(char *buf, char *lim, char **ep, char *hist) {
  char *pc = memchr(buf, '%', lim - buf);
  if (!pc) return NULL;
  memset(hist, 0, 256);

  int state = 0;
  char *bp = pc + 1;

  static const char *ok_char[] = {
    "#0- +",
    "0123456789.",
    "hlLjzt",
    "diouxXeEfFgGaAcspm%" "VJ",
    NULL
  };

  while (bp != lim && ok_char[state]) {
    if (strchr(ok_char[state], *bp)) {
      hist[(unsigned char)*bp]++;
      bp++;
      if (!ok_char[state + 1]) goto found;
    }
    else state++;
  }

  return NULL;

found:
  if (ep) *ep = bp;
  return pc;
}

static void fstash_common(jd_var *out, char *bp, char *pc) {
  if (out->type != ARRAY)
    jd_set_array(out, 30);

  if (bp != pc)
    jd_set_bytes(jd_push(out, 1), bp, pc - bp);
}

static void fstash(jd_var *out, char **bp, char *pc, char *ep, ...) {
  JD_SCOPE {
    char tmp;
    va_list ap;
    JD_VAR(frag);

    fstash_common(out, *bp, pc);

    tmp = *ep;
    *ep = '\0';

    va_start(ap, ep);
    jd_set_empty_string(frag, 100);
    safe_vprintf(jd__as_string(frag), pc, ap);
    va_end(ap);

    *ep = tmp;

    jd_assign(jd_push(out, 1), frag);
  }
  *bp = ep;
}

static void fstash_var(jd_var *out, char **bp, char *pc, char *ep, jd_var *v) {
  fstash_common(out, *bp, pc);
  jd_stringify(jd_push(out, 1), v);
  *bp = ep;
}

static void fstash_json(jd_var *out, char **bp, char *pc, char *ep, jd_var *v) {
  fstash_common(out, *bp, pc);
  jd_to_json(jd_push(out, 1), v);
  *bp = ep;
}

static void fstash_json_pretty(jd_var *out, char **bp, char *pc, char *ep, jd_var *v) {
  fstash_common(out, *bp, pc);
  jd_to_json_pretty(jd_push(out, 1), v);
  *bp = ep;
}

jd_var *jd_vsprintvf(jd_var *out, jd_var *fmt, va_list ap) {
  JD_SCOPE {
    JD_VAR(tmp);
    size_t len;
    char *fbuf = (char *) jd_bytes(fmt, &len);
    char *flim = fbuf + len - 1;
    char hist[256];

    for (;;) {
      char *ep;
      char *pc = find_sub(fbuf, flim, &ep, hist);
      if (!pc) break;
      switch (ep[-1]) {
      case '%':
      case 'm':
        fstash(tmp, &fbuf, pc, ep);
        break;
      case 'c':
      case 'd':
      case 'i':
        switch (hist['l']) {
        case 0:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, int));
          break;
        case 1:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, long int));
          break;
        default:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, long long int));
          break;
        }
        break;
      case 'o':
      case 'u':
      case 'x':
      case 'X':
        switch (hist['l']) {
        case 0:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, unsigned int));
          break;
        case 1:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, unsigned long int));
          break;
        default:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, unsigned long long int));
          break;
        }
        break;
      case 'e':
      case 'E':
      case 'f':
      case 'F':
      case 'g':
      case 'G':
      case 'a':
      case 'A':
        switch (hist['L']) {
        case 0:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, double));
          break;
        default:
          fstash(tmp, &fbuf, pc, ep, va_arg(ap, long double));
          break;
        }
        break;
      case 's':
        fstash(tmp, &fbuf, pc, ep, va_arg(ap, char *));
        break;
      case 'p':
        fstash(tmp, &fbuf, pc, ep, va_arg(ap, void *));
        break;
      case 'V':
        fstash_var(tmp, &fbuf, pc, ep, va_arg(ap, jd_var *));
        break;
      case 'J':
        switch (hist['l']) {
        case 0:
          fstash_json(tmp, &fbuf, pc, ep, va_arg(ap, jd_var *));
          break;
        default:
          fstash_json_pretty(tmp, &fbuf, pc, ep, va_arg(ap, jd_var *));
          break;
        }
        break;
      }
    }

    if (tmp->type == ARRAY) {
      if (fbuf != flim)
        jd_set_bytes(jd_push(tmp, 1), fbuf, flim - fbuf);
      jd_join(out, NULL, tmp);
    }
    else {
      /* simple case: no substitution */
      jd_assign(out, fmt);
    }
  }

  return out;
}

jd_var *jd_vsprintf(jd_var *out, const char *fmt, va_list ap) {
  JD_SCOPE {
    JD_SV(vfmt, fmt);
    jd_vsprintvf(out, vfmt, ap);
  }
  return out;
}

jd_var *jd_sprintf(jd_var *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  jd_vsprintf(out, fmt, ap);
  va_end(ap);
  return out;
}

jd_var *jd_sprintvf(jd_var *out, jd_var *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  jd_vsprintvf(out, fmt, ap);
  va_end(ap);
  return out;
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
