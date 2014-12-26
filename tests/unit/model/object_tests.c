#include <CUnit/CUnit.h>
#include <string.h>

#include "unit.h"
#include "errors.h"
#include "model/object.h"
#include "runtime/runtime.h"

static void test_immediate_integer(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_object i;
  err = seg_integer(r, 42l, &i);
  SEG_ASSERT_OK(err);

  seg_object kls = seg_object_class(r, i);
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  SEG_ASSERT_SAME(kls, boots->integer_class);

  int64_t v = 0l;
  err = seg_integer_value(i, &v);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(v, 42l);

  seg_object n;
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

static void test_immediate_string(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_object s;
  err = seg_string(r, "sup", 3, &s);
  SEG_ASSERT_OK(err);

  seg_object kls = seg_object_class(r, s);
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  SEG_ASSERT_SAME(kls, boots->string_class);

  char *v = NULL;
  uint64_t len = 0;
  err = seg_stringlike_contents(&s, &v, &len);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(len, 3);
  CU_ASSERT_EQUAL(strncmp(v, "sup", len), 0);

  seg_delete_runtime(r);
}

static void test_immediate_symbol(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_object s;
  err = seg_symbol(r, "short", 5, &s);
  SEG_ASSERT_OK(err);

  seg_object kls = seg_object_class(r, s);
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  SEG_ASSERT_SAME(kls, boots->symbol_class);

  char *v = NULL;
  uint64_t len = 0;
  err = seg_stringlike_contents(&s, &v, &len);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(len, 5);
  CU_ASSERT_EQUAL(strncmp(v, "short", len), 0);

  seg_delete_runtime(r);
}

static void test_storage(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_object imm, empty, stringlike, slotted;
  err = seg_integer(r, 42l, &imm);
  SEG_ASSERT_OK(err);

  seg_storage storage;
  err = seg_object_storage(imm, &storage);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_IMMEDIATE);

  err = seg_object_storage(empty, &storage);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_EMPTY);

  err = seg_object_storage(stringlike, &storage);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_STRINGLIKE);

  err = seg_object_storage(slotted, &storage);
  SEG_ASSERT_OK(err);
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_SLOTTED);

  seg_delete_runtime(r);
}

CU_pSuite initialize_object_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("object", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_immediate_integer);
  ADD_TEST(test_immediate_string);
  ADD_TEST(test_immediate_symbol);
  ADD_TEST(test_storage);

  return pSuite;
}
