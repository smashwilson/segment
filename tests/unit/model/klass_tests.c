#include <CUnit/CUnit.h>

#include "model/klass.h"

#include "unit.h"
#include "errors.h"
#include "runtime/runtime.h"

static void test_class(void)
{
  seg_runtime *r = NULL;
  SEG_ASSERT_TRY(seg_new_runtime(&r));

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object klass;
  SEG_ASSERT_TRY(seg_class(r, "Test", SEG_STORAGE_SLOTTED, &klass));
  SEG_ASSERT_TRY(seg_class_ivars(r, klass, 3, "one", "two", "three"));

  seg_storage storage;
  seg_object o, ivars;
  int64_t i;
  uint64_t u;
  char *n;

  SEG_ASSERT_TRY(seg_object_storage(klass, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_SLOTTED);

  SEG_ASSERT_TRY(seg_slotted_length(klass, &u));
  CU_ASSERT_EQUAL(u, SEG_CLASS_SLOTCOUNT);

  SEG_ASSERT_TRY(seg_object_class(r, klass, &o));
  SEG_ASSERT_SAME(o, boots->class_class);

  SEG_ASSERT_TRY(seg_slot_at(klass, SEG_CLASS_SLOT_NAME, &o));
  SEG_ASSERT_TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 4);
  CU_ASSERT_EQUAL(strncmp(n, "Test", u), 0);

  SEG_ASSERT_TRY(seg_slot_at(klass, SEG_CLASS_SLOT_STORAGE, &o));
  SEG_ASSERT_TRY(seg_integer_value(o, &i));
  CU_ASSERT_EQUAL(i, SEG_STORAGE_SLOTTED);

  SEG_ASSERT_TRY(seg_slot_at(klass, SEG_CLASS_SLOT_LENGTH, &o));
  SEG_ASSERT_TRY(seg_integer_value(o, &i));
  CU_ASSERT_EQUAL(i, 3);

  SEG_ASSERT_TRY(seg_slot_at(klass, SEG_CLASS_SLOT_IVARS, &ivars));
  SEG_ASSERT_TRY(seg_slotted_length(ivars, &u));
  CU_ASSERT_EQUAL(u, 3);

  SEG_ASSERT_TRY(seg_slot_at(ivars, 0, &o));
  SEG_ASSERT_TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 3);
  CU_ASSERT_EQUAL(strncmp(n, "one", u), 0);

  SEG_ASSERT_TRY(seg_slot_at(ivars, 1, &o));
  SEG_ASSERT_TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 3);
  CU_ASSERT_EQUAL(strncmp(n, "two", u), 0);

  SEG_ASSERT_TRY(seg_slot_at(ivars, 2, &o));
  SEG_ASSERT_TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 5);
  CU_ASSERT_EQUAL(strncmp(n, "three", u), 0);

  seg_delete_runtime(r);
}

CU_pSuite initialize_klass_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("klass", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_class);

  return pSuite;
}
