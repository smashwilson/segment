#include <CUnit/CUnit.h>
#include <stdint.h>
#include <stdio.h>

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

static void test_putifabsent(void)
{
  seg_ptrtable *table = seg_new_ptrtable(10L, sizeof(key));

  /* Populate the table. */

  key key0 = {0, 0};
  value value0 = {"zero"};

  key key1 = {1, 1};
  value value1 = {"one"};

  value value2 = {"newone"};

  seg_ptrtable_put(table, &key0, &value0);
  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 1);

  /* Insert a new item. */
  void *existing1 = seg_ptrtable_putifabsent(table, &key1, &value1);
  CU_ASSERT_PTR_EQUAL(existing1, &value1);
  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 2);

  /* Retrieve an existing item. */
  void *existing0 = seg_ptrtable_putifabsent(table, &key0, &value2);
  CU_ASSERT_PTR_EQUAL(existing0, &value0);
  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 2);

  void *out0 = seg_ptrtable_get(table, &key0);
  CU_ASSERT_PTR_EQUAL(out0, &value0);

  void *out1 = seg_ptrtable_get(table, &key1);
  CU_ASSERT_PTR_EQUAL(out1, &value1);

  seg_delete_ptrtable(table);
}

typedef struct {
  int correct;
  int incorrect;
} iterator_state;

static void ptrtable_iterator(const void *k, void *value, void *state)
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
    printf("Unexpected pair in ptrtable! [%d, %d] -> [%s]\n",
      thekey->aaa, thekey->bbb, (const char*) value);
    s->incorrect++;
  }
}

static void test_each(void)
{
  iterator_state s;
  s.correct = 0;
  s.incorrect = 0;

  seg_ptrtable *table = seg_new_ptrtable(10L, sizeof(key));

  key k0 = {0, 0};
  key k1 = {1, 1};
  key k2 = {2, 2};
  key k3 = {3, 3};

  seg_ptrtable_put(table, &k0, "aval");
  seg_ptrtable_put(table, &k1, "bval");
  seg_ptrtable_put(table, &k2, "cval");
  seg_ptrtable_put(table, &k3, "dval");
  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 4);

  seg_ptrtable_each(table, &ptrtable_iterator, &s);

  CU_ASSERT_EQUAL(s.correct, 4);
  CU_ASSERT_EQUAL(s.incorrect, 0);

  seg_delete_ptrtable(table);
}

static void test_resize(void)
{
  seg_ptrtable *table = seg_new_ptrtable(5L, sizeof(key));

  key k0 = { 0, 0 };
  key k1 = { 1, 1 };
  key k2 = { 2, 2 };
  key k3 = { 3, 3 };

  seg_ptrtable_put(table, &k0, "aval");
  seg_ptrtable_put(table, &k1, "bval");
  seg_ptrtable_put(table, &k2, "cval");

  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 3);
  CU_ASSERT_EQUAL(seg_ptrtable_capacity(table), 5);

  seg_ptrtable_put(table, &k3, "dval");

  CU_ASSERT_EQUAL(seg_ptrtable_count(table), 4);
  CU_ASSERT_EQUAL(seg_ptrtable_capacity(table), 10);

  void *outa = seg_ptrtable_get(table, &k0);
  CU_ASSERT_STRING_EQUAL(outa, "aval");

  void *outb = seg_ptrtable_get(table, &k1);
  CU_ASSERT_STRING_EQUAL(outb, "bval");

  void *outc = seg_ptrtable_get(table, &k2);
  CU_ASSERT_STRING_EQUAL(outc, "cval");

  void *outd = seg_ptrtable_get(table, &k3);
  CU_ASSERT_STRING_EQUAL(outd, "dval");

  seg_delete_ptrtable(table);
}

CU_pSuite initialize_ptrtable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("ptrtable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_access);
  ADD_TEST(test_putifabsent);
  ADD_TEST(test_each);
  ADD_TEST(test_resize);

  return pSuite;
}
