#include <CUnit/CUnit.h>

#include "unit.h"
#include "object.h"

static void test_integer_literal(void)
{
  seg_object *i = seg_integer(42l);
  CU_ASSERT_PTR_NOT_NULL_FATAL(i);

  CU_ASSERT_EQUAL(seg_integer_value(i), 42l);
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
  seg_object *o = seg_integer(3l);
  CU_ASSERT_FALSE(seg_isclass(o));

  seg_object *cls = seg_class(o);
  CU_ASSERT_TRUE(seg_isclass(cls));
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
