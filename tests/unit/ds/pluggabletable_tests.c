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

CU_pSuite initialize_pluggabletable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("pluggabletable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);

  return pSuite;
}
