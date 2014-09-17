#include <CUnit/CUnit.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "unit.h"
#include "ds/plugtable.h"

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
  seg_err err;
  void *out;

  seg_plugtable *table;
  err = seg_new_plugtable(10L, equals0, hash0, &table);
  SEG_ASSERT_OK(err);

  CU_ASSERT_EQUAL(seg_plugtable_count(table), 0L);

  key onek = {12, 34};
  char *value = "one";

  err = seg_plugtable_put(table, &onek, value, &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);
  CU_ASSERT_EQUAL(seg_plugtable_count(table), 1L);

  key rightk = {12, 700};
  void *out0 = seg_plugtable_get(table, &rightk);
  CU_ASSERT_PTR_EQUAL(out0, value);

  key wrongk = {13, 34};
  void *out1 = seg_plugtable_get(table, &wrongk);
  CU_ASSERT_PTR_NULL(out1);

  seg_delete_plugtable(table);
}

static void test_putifabsent(void)
{
  seg_err err;

  seg_plugtable *table;
  err = seg_new_plugtable(10L, equals0, hash0, &table);
  SEG_ASSERT_OK(err);

  /* Populate the table. */

  key key0 = {0, 0};
  char *value0 = "zero";

  key key1 = {1, 1};
  char *value1 = "one";

  char *value2 = "newone";

  void *out;
  err = seg_plugtable_put(table, &key0, value0, &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);
  CU_ASSERT_EQUAL(seg_plugtable_count(table), 1);

  /* Insert a new item. */
  void *existing1;
  err = seg_plugtable_putifabsent(table, &key1, value1, &existing1);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_EQUAL(existing1, value1);
  CU_ASSERT_EQUAL(seg_plugtable_count(table), 2);

  /* Retrieve an existing item. */
  void *existing0;
  err = seg_plugtable_putifabsent(table, &key0, value2, &existing0);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_EQUAL(existing0, value0);
  CU_ASSERT_EQUAL(seg_plugtable_count(table), 2);

  void *out0 = seg_plugtable_get(table, &key0);
  CU_ASSERT_PTR_EQUAL(out0, value0);

  void *out1 = seg_plugtable_get(table, &key1);
  CU_ASSERT_PTR_EQUAL(out1, value1);

  seg_delete_plugtable(table);
}

typedef struct {
  int correct;
  int incorrect;
} iterator_state;

static seg_err ptrtable_iterator(const void *k, void *value, void *state)
{
  key *thekey = (key *) k;
  iterator_state *s = (iterator_state *) state;

  if (thekey->aaa == 0 && thekey->bbb == 0 && ! memcmp(value, "aval", 4)) {
    s->correct++;
  } else if (thekey->aaa == 1 && thekey->bbb == 1 && ! memcmp(value, "bval", 4)) {
    s->correct++;
  } else if (thekey->aaa == 2 && thekey->bbb == 2 && ! memcmp(value, "cval", 4)) {
    s->correct++;
  } else if (thekey->aaa == 3 && thekey->bbb == 3 && ! memcmp(value, "dval", 4)) {
    s->correct++;
  } else {
    printf("Unexpected pair in plugtable! [%d, %d] -> [%s]\n",
      thekey->aaa, thekey->bbb, (const char*) value);
    s->incorrect++;
  }

  return SEG_OK;
}

static void test_each(void)
{
  seg_err err;
  iterator_state s;
  s.correct = 0;
  s.incorrect = 0;

  seg_plugtable *table;
  err = seg_new_plugtable(10L, equals0, hash0, &table);
  SEG_ASSERT_OK(err);

  key k0 = {0, 0};
  key k1 = {1, 1};
  key k2 = {2, 2};
  key k3 = {3, 3};

  void *out;
  err = seg_plugtable_put(table, &k0, "aval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);
  err = seg_plugtable_put(table, &k1, "bval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);
  err = seg_plugtable_put(table, &k2, "cval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);
  err = seg_plugtable_put(table, &k3, "dval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);

  CU_ASSERT_EQUAL(seg_plugtable_count(table), 4);

  err = seg_plugtable_each(table, &ptrtable_iterator, &s);
  SEG_ASSERT_OK(err);

  CU_ASSERT_EQUAL(s.correct, 4);
  CU_ASSERT_EQUAL(s.incorrect, 0);

  seg_delete_plugtable(table);
}

static void test_resize(void)
{
  seg_err err;

  seg_plugtable *table;
  err = seg_new_plugtable(5L, equals0, hash0, &table);
  SEG_ASSERT_OK(err);

  key k0 = { 0, 0 };
  key k1 = { 1, 1 };
  key k2 = { 2, 2 };
  key k3 = { 3, 3 };

  void *out;
  err = seg_plugtable_put(table, &k0, "aval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);
  err = seg_plugtable_put(table, &k1, "bval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);
  err = seg_plugtable_put(table, &k2, "cval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);

  CU_ASSERT_EQUAL(seg_plugtable_count(table), 3);
  CU_ASSERT_EQUAL(seg_plugtable_capacity(table), 5);

  err = seg_plugtable_put(table, &k3, "dval", &out);
  SEG_ASSERT_OK(err);
  CU_ASSERT_PTR_NULL(out);

  CU_ASSERT_EQUAL(seg_plugtable_count(table), 4);
  CU_ASSERT_EQUAL(seg_plugtable_capacity(table), 10);

  void *outa = seg_plugtable_get(table, &k0);
  CU_ASSERT_STRING_EQUAL(outa, "aval");

  void *outb = seg_plugtable_get(table, &k1);
  CU_ASSERT_STRING_EQUAL(outb, "bval");

  void *outc = seg_plugtable_get(table, &k2);
  CU_ASSERT_STRING_EQUAL(outc, "cval");

  void *outd = seg_plugtable_get(table, &k3);
  CU_ASSERT_STRING_EQUAL(outd, "dval");

  seg_delete_plugtable(table);
}

CU_pSuite initialize_plugtable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("plugtable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);
  ADD_TEST(test_putifabsent);
  ADD_TEST(test_each);
  ADD_TEST(test_resize);

  return pSuite;
}
