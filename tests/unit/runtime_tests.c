#include <CUnit/CUnit.h>

#include "unit.h"
#include "runtime.h"

static void test_creation(void)
{
  CU_FAIL("pending");
}

CU_pSuite initialize_runtime_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("runtime", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_creation);

  return pSuite;
}
