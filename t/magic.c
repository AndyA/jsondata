/* t/magic.c */

#include <stdio.h>
#include <string.h>

#include "framework.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"
#include "jd_private.h"

void test_main(void) {
  scope {
    JD_SV(penn, "I got the magic");

    {
      jd_var *m = jd__get_magic(penn, 0x1234);
      not_null(m, "got magic");
      is(m->type, VOID, "and it's empty");
      jd_from_jsons(m, "{\"hat\":\"rabbit\"}");
    }

    {
      jd_var *m = jd__get_magic(penn, 0x4321);
      not_null(m, "got more magic");
      is(m->type, VOID, "and it's empty");
      jd_set_string(m, "More magic");
    }

    {
      jd_var *m = jd__get_magic(penn, 0x1234);
      not_null(m, "got magic again");
      jdt_is_json(m, "{\"hat\":\"rabbit\"}", "and it matches");
    }

    {
      jd_var *m = jd__get_magic(penn, 0x4321);
      not_null(m, "got more magic again");
      jdt_is_json(m, "\"More magic\"", "and it matches");
    }
  }
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
