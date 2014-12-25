#include <CUnit/CUnit.h>
#include <string.h>

#include "unit.h"
#include "errors.h"
#include "model/object.h"
#include "runtime/runtime.h"

static void test_integer_literal(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_object *i = NULL;
  err = seg_integer(r, 42l, &i);
  SEG_ASSERT_OK(err);

  seg_object *kls = seg_class(r, i);
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  CU_ASSERT_PTR_EQUAL(kls, boots->integer_class);

  int64_t v = 0l;
  err = seg_integer_value(i, &v);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(v, 42l);

  seg_object *n = NULL;
  err = seg_integer(r, -32l, &n);
  SEG_ASSERT_OK(err);

  err = seg_integer_value(n, &v);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(v, -32l);

  err = seg_integer(r, 1ll << 63, &i);
  CU_ASSERT_PTR_NOT_NULL_FATAL(err);
  CU_ASSERT_EQUAL(err->code, SEG_CODE_RANGE);

  seg_delete_runtime(r);
}

static void test_string_literal(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_object *s = NULL;
  err = seg_string(r, "sup", 3, &s);
  SEG_ASSERT_OK(err);

  char *v = NULL;
  uint64_t len = 0;
  err = seg_string_contents(s, &v, &len);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(len, 3);
  CU_ASSERT_EQUAL(strncmp(v, "sup", len), 0);

  seg_delete_runtime(r);
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
