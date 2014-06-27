#include <CUnit/CUnit.h>

#include "unit.h"

#include "symboltable.h"
#include "ds/hashtable.h"

static void test_access(void)
{
  seg_symboltablep table = seg_new_symboltable();

  seg_symbol *a0sym = seg_symboltable_intern(table, "aaa", 3);
  seg_symbol *b0sym = seg_symboltable_intern(table, "bbb", 3);
  CU_ASSERT_PTR_NOT_EQUAL(a0sym, b0sym);

  seg_symbol *b1sym = seg_symboltable_intern(table, "bbb", 3);
  CU_ASSERT_PTR_EQUAL(b0sym, b1sym);

  seg_delete_symboltable(table);
}

CU_pSuite initialize_symboltable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("symboltable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);

  return pSuite;
}
