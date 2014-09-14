#include <CUnit/CUnit.h>

#include "unit.h"
#include "errors.h"
#include "model/object.h"

static void test_integer_literal(void)
{
  seg_err result;

  seg_runtime *r = seg_new_runtime();
  CU_ASSERT_PTR_NOT_NULL_FATAL(r);

  seg_object *i = NULL;
  result = seg_integer(42l, &i);
  CU_ASSERT_EQUAL_FATAL(result, SEG_OK);

  seg_object *kls = seg_class(i, r);
  CU_ASSERT_PTR_EQUAL(kls, r.integer_class);

  int64_t v = 0l;
  result = seg_integer_value(i, &v);
  CU_ASSERT_EQUAL_FATAL(result, SEG_OK);
  CU_ASSERT_EQUAL(v, 42l);

  seg_object *n = NULL;
  result = seg_integer(-32l, &n);
  CU_ASSERT_EQUAL_FATAL(result, SEG_OK);

  result = seg_integer(1 << 63, &i);
  CU_ASSERT_EQUAL(result, SEG_RANGE);

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
