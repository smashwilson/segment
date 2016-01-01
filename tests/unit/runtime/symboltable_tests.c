#include <CUnit/CUnit.h>

#include "unit.h"

#include "model/errors.h"
#include "runtime/runtime.h"
#include "runtime/symboltable.h"
#include "model/object.h"
#include "ds/hashtable.h"

static void test_access(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_symboltable *table = seg_runtime_symboltable(r);
  uint64_t init_count = seg_symboltable_count(table);

  seg_object a0sym;
  err = seg_symboltable_cintern(table, "aaaaaaaaa", &a0sym);
  SEG_ASSERT_OK(err);

  seg_object b0sym;
  err = seg_symboltable_cintern(table, "bbbbbbbbb", &b0sym);
  SEG_ASSERT_OK(err);

  SEG_ASSERT_DIFFERENT(a0sym, b0sym);

  seg_object b1sym;
  err = seg_symboltable_cintern(table, "bbbbbbbbb", &b1sym);
  SEG_ASSERT_OK(err);

  SEG_ASSERT_SAME(b0sym, b1sym);

  uint64_t count = seg_symboltable_count(table);
  CU_ASSERT_EQUAL(count - init_count, 2);

  seg_delete_runtime(r);
}

static void test_get(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_symboltable *table = seg_runtime_symboltable(r);
  uint64_t init_count = seg_symboltable_count(table);

  seg_object a0sym;
  err = seg_symboltable_intern(table, "aaaaaaaaa", 9, &a0sym);
  SEG_ASSERT_OK(err);

  seg_object a1sym = seg_symboltable_get(table, "aaaaaaaaa", 9);
  SEG_ASSERT_SAME(a0sym, a1sym);

  seg_object b0sym = seg_symboltable_get(table, "bbbbbbbbb", 9);
  SEG_ASSERT_SAME(b0sym, SEG_NO_SYMBOL);

  uint64_t count = seg_symboltable_count(table);
  CU_ASSERT_EQUAL(count - init_count, 1);

  seg_delete_runtime(r);
}

static void test_immediate(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  err = seg_new_runtime(&r);
  SEG_ASSERT_OK(err);

  seg_symboltable *table = seg_runtime_symboltable(r);
  uint64_t init_count = seg_symboltable_count(table);

  seg_object a0sym;
  err = seg_symboltable_intern(table, "aaa", 3, &a0sym);
  SEG_ASSERT_OK(err);
  CU_ASSERT(SEG_IS_IMMEDIATE(a0sym));

  uint64_t count = seg_symboltable_count(table);
  CU_ASSERT_EQUAL(count - init_count, 0);

  seg_object a1sym = seg_symboltable_get(table, "aaa", 3);
  CU_ASSERT(SEG_IS_IMMEDIATE(a1sym));
  SEG_ASSERT_SAME(a0sym, a1sym);

  seg_object b0sym = seg_symboltable_get(table, "bbb", 3);
  CU_ASSERT(SEG_IS_IMMEDIATE(b0sym));
  SEG_ASSERT_DIFFERENT(b0sym, a0sym);

  seg_delete_runtime(r);
}

CU_pSuite initialize_symboltable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("symboltable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);
  ADD_TEST(test_get);
  ADD_TEST(test_immediate);

  return pSuite;
}
