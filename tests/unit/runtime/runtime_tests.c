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

  seg_symboltable *symtable = seg_runtime_symboltable(r);
  CU_ASSERT_PTR_NOT_NULL(symtable);

  seg_delete_runtime(r);
}

static void test_bootstrap(void)
{
  seg_runtime *r = NULL;
  SEG_ASSERT_TRY(seg_new_runtime(&r));

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  /* The Class class has itself as its class. */
  seg_object out;
  SEG_ASSERT_TRY(seg_object_class(r, boots->class_class, &out));
  SEG_ASSERT_SAME(boots->class_class, out);

  /* The other classes are instances of the Class class for each literal. */
  SEG_ASSERT_TRY(seg_object_class(r, boots->integer_class, &out));
  SEG_ASSERT_SAME(boots->class_class, out);

  SEG_ASSERT_TRY(seg_object_class(r, boots->string_class, &out));
  SEG_ASSERT_SAME(boots->class_class, out);

  SEG_ASSERT_TRY(seg_object_class(r, boots->symbol_class, &out));
  SEG_ASSERT_SAME(boots->class_class, out);

  SEG_ASSERT_TRY(seg_object_class(r, boots->block_class, &out));
  SEG_ASSERT_SAME(boots->class_class, out);

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
