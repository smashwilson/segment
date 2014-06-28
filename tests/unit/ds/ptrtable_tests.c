#include <CUnit/CUnit.h>

#include "unit.h"
#include "ds/hashtable.h"

static void test_access(void)
{
  CU_FAIL("pending");
}

CU_pSuite initialize_ptrtable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("ptrtable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);

  return pSuite;
}
