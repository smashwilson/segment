#include <CUnit/CUnit.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "unit.h"
#include "ds/pluggabletable.h"

typedef struct {
  int32_t aaa;
  int32_t bbb;
} key;

static bool equals0(const void *left, const void *right)
{
  key *lkey = (key*) left;
  key *rkey = (key*) right;

  return lkey->aaa == rkey->aaa;
}

static uint32_t hash0(const void *k)
{
  key *casted = (key*) k;

  return (uint32_t) casted->aaa;
}

static void test_access(void)
{
  seg_pluggabletable *table = seg_new_pluggabletable(10L, equals0, hash0);

  CU_ASSERT_EQUAL(seg_pluggabletable_count(table), 0L);

  key onek = {12, 34};
  char *value = "one";

  seg_pluggabletable_put(table, &onek, value);
  CU_ASSERT_EQUAL(seg_pluggabletable_count(table), 1L);

  key rightk = {12, 700};
  void *out0 = seg_pluggabletable_get(table, &rightk);
  CU_ASSERT_PTR_EQUAL(out0, value);

  key wrongk = {13, 34};
  void *out1 = seg_pluggabletable_get(table, &wrongk);
  CU_ASSERT_PTR_NULL(out1);

  seg_delete_pluggabletable(table);
}

static void test_putifabsent(void)
{
  seg_pluggabletable *table = seg_new_pluggabletable(10L, equals0, hash0);

  /* Populate the table. */

  key key0 = {0, 0};
  char *value0 = "zero";

  key key1 = {1, 1};
  char *value1 = "one";

  char *value2 = "newone";

  seg_pluggabletable_put(table, &key0, value0);
  CU_ASSERT_EQUAL(seg_pluggabletable_count(table), 1);

  /* Insert a new item. */
  void *existing1 = seg_pluggabletable_putifabsent(table, &key1, value1);
  CU_ASSERT_PTR_EQUAL(existing1, value1);
  CU_ASSERT_EQUAL(seg_pluggabletable_count(table), 2);

  /* Retrieve an existing item. */
  void *existing0 = seg_pluggabletable_putifabsent(table, &key0, value2);
  CU_ASSERT_PTR_EQUAL(existing0, value0);
  CU_ASSERT_EQUAL(seg_pluggabletable_count(table), 2);

  void *out0 = seg_pluggabletable_get(table, &key0);
  CU_ASSERT_PTR_EQUAL(out0, value0);

  void *out1 = seg_pluggabletable_get(table, &key1);
  CU_ASSERT_PTR_EQUAL(out1, value1);

  seg_delete_pluggabletable(table);
}

CU_pSuite initialize_pluggabletable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("pluggabletable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);
  ADD_TEST(test_putifabsent);

  return pSuite;
}
