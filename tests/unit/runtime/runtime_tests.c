#include <CUnit/CUnit.h>

#include "unit.h"
#include "runtime/runtime.h"
#include "runtime/symboltable.h"
#include "model/object.h"

static void test_creation(void)
{
  seg_err err;

  seg_runtime *r;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  printf("r = %p\n", r);

  seg_symboltable *symtable = seg_runtime_symboltable(r);
  CU_ASSERT_PTR_NOT_NULL(symtable);

  seg_delete_runtime(r);
}

static void test_bootstrap(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  /* The Class class has itself as its class. */
  SEG_ASSERT_SAME(boots->class_class, seg_class(r, boots->class_class));

  /* The other classes are instances of the Class class for each literal. */
  SEG_ASSERT_SAME(boots->class_class, seg_class(r, boots->integer_class));
  SEG_ASSERT_SAME(boots->class_class, seg_class(r, boots->string_class));
  SEG_ASSERT_SAME(boots->class_class, seg_class(r, boots->symbol_class));
  SEG_ASSERT_SAME(boots->class_class, seg_class(r, boots->block_class));

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
