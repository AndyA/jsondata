/* object.t */

#include <stdio.h>

#include "util.h"
#include "tap.h"
#include "jd_test.h"
#include "jd_pretty.h"

static int freed = 0;

static void my_free(void *p) {
  freed++;
  jd_free(p);
}

void test_main(void) {
  jd_var obj1 = JD_INIT, obj2 = JD_INIT, obj3 = JD_INIT;

  scope {
    jd_set_object(&obj1, jd_alloc(10), my_free);
    freed = 0;

    jd_clone(&obj2, &obj1, 1);
    jd_assign(&obj3, &obj2);

    jd_release(&obj1);
    is(freed, 0, "release obj1 - nothing freed");
    jd_release(&obj2);
    is(freed, 0, "release obj2 - nothing freed");
    jd_release(&obj3);
    is(freed, 1, "release obj3 - freed");
  }
}

/* vim:ts=2:sw=2:sts=2:et:ft=c
 */
