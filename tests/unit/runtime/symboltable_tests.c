#include <CUnit/CUnit.h>

#include "unit.h"

#include "errors.h"
#include "runtime/symboltable.h"
#include "model/object.h"
#include "ds/hashtable.h"

static void test_access(void)
{
  seg_err err;

  seg_symboltable *table = NULL;
  err = seg_new_symboltable(&table);
  SEG_ASSERT_OK(err);

  seg_object *a0sym = NULL;
  err = seg_symboltable_intern(table, "aaa", 3, &a0sym);
  SEG_ASSERT_OK(err);

  seg_object *b0sym = NULL;
  err = seg_symboltable_intern(table, "bbb", 3, &b0sym);
  SEG_ASSERT_OK(err);

  CU_ASSERT_PTR_NOT_EQUAL(a0sym, b0sym);

  seg_object *b1sym = NULL;
  err = seg_symboltable_intern(table, "bbb", 3, &b1sym);
  SEG_ASSERT_OK(err);

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
