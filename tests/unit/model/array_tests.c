#include <stdint.h>
#include <CUnit/CUnit.h>

#include "model/array.h"

#include "unit.h"
#include "model/errors.h"
#include "model/object.h"
#include "runtime/runtime.h"

static void test_empty_array(void)
{
  seg_runtime *r = NULL;
  SEG_ASSERT_TRY(seg_new_runtime(&r));
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object empty;
  SEG_ASSERT_TRY(seg_empty_array(r, 0, &empty));

  seg_object klass;
  SEG_ASSERT_TRY(seg_object_class(r, empty, &klass));
  SEG_ASSERT_SAME(klass, boots->array_class);

  seg_storage storage;
  SEG_ASSERT_TRY(seg_object_storage(empty, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_SLOTTED);

  uint64_t capacity, length;
  SEG_ASSERT_TRY(seg_array_capacity(empty, &capacity));
  SEG_ASSERT_TRY(seg_array_length(empty, &length));
  CU_ASSERT_EQUAL(capacity, 0);
  CU_ASSERT_EQUAL(length, 0);

  seg_delete_runtime(r);
}

CU_pSuite initialize_array_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("array", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_empty_array);

  return pSuite;
}
