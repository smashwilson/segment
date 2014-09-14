#include <CUnit/CUnit.h>

#include "unit.h"
#include "runtime/runtime.h"
#include "runtime/symboltable.h"
#include "model/object.h"

static void test_creation(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  CU_ASSERT_EQUAL_FATAL(err, SEG_OK);

  seg_symboltable *symtable = seg_runtime_symboltable(r);
  CU_ASSERT_PTR_NOT_NULL(symtable);

  seg_delete_runtime(r);
}

static void test_bootstrap(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  CU_ASSERT_EQUAL_FATAL(err, SEG_OK);

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  /* The Class class has itself as its class. */
  CU_ASSERT_EQUAL(boots->class_class, seg_class(boots->class_class, r));

  /* The other classes are instances of the Class class for each literal. */
  CU_ASSERT_EQUAL(boots->class_class, seg_class(boots->integer_class, r));
  CU_ASSERT_EQUAL(boots->class_class, seg_class(boots->string_class, r));
  CU_ASSERT_EQUAL(boots->class_class, seg_class(boots->symbol_class, r));
  CU_ASSERT_EQUAL(boots->class_class, seg_class(boots->block_class, r));

  seg_delete_runtime(r);
}

CU_pSuite initialize_runtime_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("runtime", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_creation);
  ADD_TEST(test_bootstrap);

  return pSuite;
}
