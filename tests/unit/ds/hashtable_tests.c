#include "hashtable_tests.h"

#include "../../../src/ds/hashtable.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_hashtable_access(void **state)
{
  seg_hashtablep table = seg_new_hashtable(10L);

  const char *key0 = "somekey";
  const char *value0 = "somevalue";

  const char *key1 = "otherkey";
  const char *value1 = "othervalue";

  seg_hashtable_put(table, key0, value0);

  const char *out0 = seg_hashtable_get(table, key0);
}
