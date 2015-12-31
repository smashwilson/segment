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
  SEG_ASSERT_TRY(seg_new_runtime(&r));

  seg_object i;
  SEG_ASSERT_TRY(seg_integer(r, 42l, &i));

  seg_object kls;
  SEG_ASSERT_TRY(seg_object_class(r, i, &kls));
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  SEG_ASSERT_SAME(kls, boots->integer_class);

  int64_t v = 0l;
  SEG_ASSERT_TRY(seg_integer_value(i, &v));
  CU_ASSERT_EQUAL(v, 42l);

  seg_object n;
  SEG_ASSERT_TRY(seg_integer(r, -32l, &n));

  SEG_ASSERT_TRY(seg_integer_value(n, &v));
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
  SEG_ASSERT_TRY(seg_new_runtime(&r));
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object s;
  SEG_ASSERT_TRY(seg_string(r, "sup", 3, &s));

  seg_object kls;
  SEG_ASSERT_TRY(seg_object_class(r, s, &kls));
  SEG_ASSERT_SAME(kls, boots->string_class);

  char *v = NULL;
  uint64_t len = 0;
  SEG_ASSERT_TRY(seg_buffer_contents(&s, &v, &len));
  CU_ASSERT_EQUAL(len, 3);
  CU_ASSERT_EQUAL(strncmp(v, "sup", len), 0);

  seg_delete_runtime(r);
}

static void test_immediate_symbol(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  SEG_ASSERT_TRY(seg_new_runtime(&r));
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object s;
  SEG_ASSERT_TRY(seg_symbol(r, "short", 5, &s));

  seg_object kls;
  SEG_ASSERT_TRY(seg_object_class(r, s, &kls));
  SEG_ASSERT_SAME(kls, boots->symbol_class);

  char *v = NULL;
  uint64_t len = 0;
  SEG_ASSERT_TRY(seg_buffer_contents(&s, &v, &len));
  CU_ASSERT_EQUAL(len, 5);
  CU_ASSERT_EQUAL(strncmp(v, "short", len), 0);

  seg_delete_runtime(r);
}

static void test_slotted(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  SEG_ASSERT_TRY(seg_new_runtime(&r));

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object klass;
  SEG_ASSERT_TRY(seg_class(r, "SlottedClass", SEG_STORAGE_SLOTTED, &klass));
  SEG_ASSERT_TRY(seg_class_ivars(r, klass, 4, "aa", "bb", "cc", "dd"));

  seg_object instance;
  SEG_ASSERT_TRY(seg_slotted(r, klass, &instance));

  uint64_t len;
  SEG_ASSERT_TRY(seg_slotted_length(instance, &len));
  CU_ASSERT_EQUAL(len, 4);

  seg_object klass_out;
  SEG_ASSERT_TRY(seg_object_class(r, instance, &klass_out));
  SEG_ASSERT_SAME(klass_out, klass);

  seg_storage storage;
  SEG_ASSERT_TRY(seg_object_storage(instance, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_SLOTTED);

  for (int i = 0; i < len; i++) {
    seg_object slot;
    SEG_ASSERT_TRY(seg_slot_at(instance, i, &slot));
    SEG_ASSERT_SAME(slot, boots->none_instance);
  }

  seg_delete_runtime(r);
}

static void test_storage(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  SEG_ASSERT_TRY(seg_new_runtime(&r));

  seg_object imm, buffer, slotted;
  SEG_ASSERT_TRY(seg_integer(r, 42l, &imm));

  seg_storage storage;
  SEG_ASSERT_TRY(seg_object_storage(imm, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_IMMEDIATE);

  SEG_ASSERT_TRY(seg_object_storage(buffer, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_BUFFER);

  SEG_ASSERT_TRY(seg_object_storage(slotted, &storage));
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
  ADD_TEST(test_class);
  ADD_TEST(test_slotted);
  ADD_TEST(test_storage);

  return pSuite;
}
