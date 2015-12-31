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
  TRY(seg_new_runtime(&r));

  seg_object i;
  TRY(seg_integer(r, 42l, &i));

  seg_object kls;
  TRY(seg_object_class(r, i, &kls));
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  SEG_ASSERT_SAME(kls, boots->integer_class);

  int64_t v = 0l;
  TRY(seg_integer_value(i, &v));
  CU_ASSERT_EQUAL(v, 42l);

  seg_object n;
  TRY(seg_integer(r, -32l, &n));

  TRY(seg_integer_value(n, &v));
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
  TRY(seg_new_runtime(&r));
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object s;
  TRY(seg_string(r, "sup", 3, &s));

  seg_object kls;
  TRY(seg_object_class(r, s, &kls));
  SEG_ASSERT_SAME(kls, boots->string_class);

  char *v = NULL;
  uint64_t len = 0;
  TRY(seg_buffer_contents(&s, &v, &len));
  CU_ASSERT_EQUAL(len, 3);
  CU_ASSERT_EQUAL(strncmp(v, "sup", len), 0);

  seg_delete_runtime(r);
}

static void test_immediate_symbol(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  TRY(seg_new_runtime(&r));
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object s;
  TRY(seg_symbol(r, "short", 5, &s));

  seg_object kls;
  TRY(seg_object_class(r, s, &kls));
  SEG_ASSERT_SAME(kls, boots->symbol_class);

  char *v = NULL;
  uint64_t len = 0;
  TRY(seg_buffer_contents(&s, &v, &len));
  CU_ASSERT_EQUAL(len, 5);
  CU_ASSERT_EQUAL(strncmp(v, "short", len), 0);

  seg_delete_runtime(r);
}

static void test_class(void)
{
  seg_runtime *r = NULL;
  TRY(seg_new_runtime(&r));

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object klass;
  TRY(seg_class(r, "Test", SEG_STORAGE_SLOTTED, &klass));
  TRY(seg_class_ivars(r, klass, 3, "one", "two", "three"));

  seg_storage storage;
  seg_object o, ivars;
  int64_t i;
  uint64_t u;
  char *n;

  TRY(seg_object_storage(klass, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_SLOTTED);

  TRY(seg_slotted_length(klass, &u));
  CU_ASSERT_EQUAL(u, SEG_CLASS_SLOTCOUNT);

  TRY(seg_object_class(r, klass, &o));
  SEG_ASSERT_SAME(o, boots->class_class);

  TRY(seg_slot_at(klass, SEG_CLASS_SLOT_NAME, &o));
  TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 4);
  CU_ASSERT_EQUAL(strncmp(n, "Test", u), 0);

  TRY(seg_slot_at(klass, SEG_CLASS_SLOT_STORAGE, &o));
  TRY(seg_integer_value(o, &i));
  CU_ASSERT_EQUAL(i, SEG_STORAGE_SLOTTED);

  TRY(seg_slot_at(klass, SEG_CLASS_SLOT_SLOTS, &o));
  TRY(seg_integer_value(o, &i));
  CU_ASSERT_EQUAL(i, 3);

  TRY(seg_slot_at(klass, SEG_CLASS_SLOT_IVARS, &ivars));
  TRY(seg_slotted_length(ivars, &u));
  CU_ASSERT_EQUAL(u, 3);

  TRY(seg_slot_at(ivars, 0, &o));
  TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 3);
  CU_ASSERT_EQUAL(strncmp(n, "one", u), 0);

  TRY(seg_slot_at(ivars, 1, &o));
  TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 3);
  CU_ASSERT_EQUAL(strncmp(n, "two", u), 0);

  TRY(seg_slot_at(ivars, 2, &o));
  TRY(seg_buffer_contents(&o, &n, &u));
  CU_ASSERT_EQUAL(u, 5);
  CU_ASSERT_EQUAL(strncmp(n, "three", u), 0);

  seg_delete_runtime(r);
}

static void test_slotted(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  TRY(seg_new_runtime(&r));

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object klass;
  TRY(seg_class(r, "SlottedClass", SEG_STORAGE_SLOTTED, &klass));
  TRY(seg_class_ivars(r, klass, 4, "aa", "bb", "cc", "dd"));

  seg_object instance;
  TRY(seg_slotted(r, klass, &instance));

  uint64_t len;
  TRY(seg_slotted_length(instance, &len));
  CU_ASSERT_EQUAL(len, 4);

  seg_object klass_out;
  TRY(seg_object_class(r, instance, &klass_out));
  SEG_ASSERT_SAME(klass_out, klass);

  seg_storage storage;
  TRY(seg_object_storage(instance, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_SLOTTED);

  for (int i = 0; i < len; i++) {
    seg_object slot;
    TRY(seg_slot_at(instance, i, &slot));
    SEG_ASSERT_SAME(slot, boots->none_instance);
  }

  seg_delete_runtime(r);
}

static void test_storage(void)
{
  seg_err err;

  seg_runtime *r = NULL;
  TRY(seg_new_runtime(&r));

  seg_object imm, buffer, slotted;
  TRY(seg_integer(r, 42l, &imm));

  seg_storage storage;
  TRY(seg_object_storage(imm, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_IMMEDIATE);

  TRY(seg_object_storage(buffer, &storage));
  CU_ASSERT_EQUAL(storage, SEG_STORAGE_BUFFER);

  TRY(seg_object_storage(slotted, &storage));
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
