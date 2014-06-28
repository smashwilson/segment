#include <CUnit/CUnit.h>
#include <stdint.h>

#include "unit.h"
#include "ds/ptrtable.h"

typedef struct {
  int32_t aaa;
  int32_t bbb;
} key;

typedef struct {
  const char *name;
} value;

static void test_access(void)
{
  seg_ptrtable *table = seg_new_ptrtable(10L, sizeof(key));

  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 0L);

  key onek = {12, 34};
  value onev = {"one"};
  seg_ptrtable_put(table, &onek, &onev);
  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 1L);

  key rightk = {12, 34};
  value *out0 = seg_ptrtable_get(table, &rightk);
  CU_ASSERT_PTR_EQUAL(&onev, out0);

  key wrongk = {12, 56};
  value *out1 = seg_ptrtable_get(table, &wrongk);
  CU_ASSERT_PTR_NULL(out1);

  seg_delete_ptrtable(table);
}

CU_pSuite initialize_ptrtable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("ptrtable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);

  return pSuite;
}
