#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <string.h>

#include "hashtable_tests.h"
#include "../unit.h"

#include "../../../src/ds/hashtable.h"

void test_hashtable_access(void)
{
  seg_hashtablep table = seg_new_hashtable(10L);

  const char *key0 = "somekey";
  size_t length0 = strlen(key0);
  char *value0 = "somevalue";

  const char *key1 = "otherkey";
  size_t length1 = strlen(key1);

  seg_hashtable_put(table, key0, length0, value0);

  const char *out0 = seg_hashtable_get(table, key0, length0);
  CU_ASSERT_PTR_EQUAL(out0, value0);

  const char *out1 = seg_hashtable_get(table, key1, length1);
  CU_ASSERT_PTR_NULL(out1);

  seg_delete_hashtable(table);
}

CU_pSuite initialize_hashtable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("hashtable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_hashtable_access);

  return pSuite;
}
