#include <CUnit/CUnit.h>

#include "unit.h"

#include "errors.h"
#include "runtime/runtime.h"
#include "runtime/symboltable.h"
#include "model/object.h"
#include "ds/hashtable.h"

static void test_access(void)
{
  seg_err err;

  seg_runtime *runtime = NULL;
  err = seg_new_runtime(&runtime);
  SEG_ASSERT_OK(err);

  seg_symboltable *table = seg_runtime_symboltable(runtime);

  seg_object a0sym;
  err = seg_symboltable_intern(table, "aaa", 3, &a0sym);
  SEG_ASSERT_OK(err);

  seg_object b0sym;
  err = seg_symboltable_intern(table, "bbb", 3, &b0sym);
  SEG_ASSERT_OK(err);

  SEG_ASSERT_DIFFERENT(a0sym, b0sym);

  seg_object b1sym;
  err = seg_symboltable_intern(table, "bbb", 3, &b1sym);
  SEG_ASSERT_OK(err);

  SEG_ASSERT_SAME(b0sym, b1sym);

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
