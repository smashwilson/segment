#include <CUnit/CUnit.h>

#include "unit.h"
#include "runtime.h"
#include "symboltable.h"

static void test_creation(void)
{
  seg_runtime *r = seg_new_runtime();
  CU_ASSERT_PTR_NOT_NULL_FATAL(r);

  seg_symboltable *symtable = seg_runtime_symboltable(r);
  CU_ASSERT_PTR_NOT_NULL(symtable);

  seg_delete_runtime(r);
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
