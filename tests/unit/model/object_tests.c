#include <CUnit/CUnit.h>

#include "unit.h"
#include "errors.h"
#include "model/object.h"
#include "runtime/runtime.h"

static void test_integer_literal(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  CU_ASSERT_EQUAL_FATAL(err, SEG_OK);

  seg_object *i = NULL;
  err = seg_integer(42l, &i);
  CU_ASSERT_EQUAL_FATAL(err, SEG_OK);

  seg_object *kls = seg_class(i, r);
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  CU_ASSERT_PTR_EQUAL(kls, boots->integer_class);

  int64_t v = 0l;
  err = seg_integer_value(i, &v);
  CU_ASSERT_EQUAL_FATAL(err, SEG_OK);
  CU_ASSERT_EQUAL(v, 42l);

  seg_object *n = NULL;
  err = seg_integer(-32l, &n);
  CU_ASSERT_EQUAL_FATAL(err, SEG_OK);

  err = seg_integer(1 << 63, &i);
  CU_ASSERT_EQUAL(err, SEG_RANGE);

  seg_delete_runtime(r);
}

static void test_string_literal(void)
{
  CU_FAIL("pending");
}

static void test_symbol_literal(void)
{
  CU_FAIL("pending");
}

static void test_isclass(void)
{
  CU_FAIL("pending");
}

CU_pSuite initialize_object_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("object", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_integer_literal);
  ADD_TEST(test_string_literal);
  ADD_TEST(test_symbol_literal);
  ADD_TEST(test_isclass);

  return pSuite;
}