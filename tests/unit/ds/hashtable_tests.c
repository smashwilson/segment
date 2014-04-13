#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <string.h>
#include <stdio.h>

#include "hashtable_tests.h"
#include "../unit.h"

#include "../../../src/ds/hashtable.h"

void test_hashtable_access(void)
{
  seg_hashtablep table = seg_new_hashtable(10L);

  const char *key0 = "somekey";
  size_t length0 = strlen(key0);
  char *value0 = "somevalue";

  const char *key1 = "otherkey";
  size_t length1 = strlen(key1);

  CU_ASSERT_EQUAL(seg_hashtable_count(table), 0);

  seg_hashtable_put(table, key0, length0, value0);
  CU_ASSERT_EQUAL(seg_hashtable_count(table), 1);

  const char *out0 = seg_hashtable_get(table, key0, length0);
  CU_ASSERT_PTR_EQUAL(out0, value0);

  const char *out1 = seg_hashtable_get(table, key1, length1);
  CU_ASSERT_PTR_NULL(out1);

  seg_delete_hashtable(table);
}

void test_hashtable_putifabsent(void)
{
  seg_hashtablep table = seg_new_hashtable(10L);

  /* Populate the table. */

  const char *key0 = "somekey";
  size_t length0 = strlen(key0);
  char *value0 = "somevalue";

  const char *key1 = "otherkey";
  size_t length1 = strlen(key1);
  char *value1 = "othervalue";

  char *value01 = "newvalue";

  seg_hashtable_put(table, key0, length0, value0);
  CU_ASSERT_EQUAL(seg_hashtable_count(table), 1);

  /* Insert a new item. */
  void *existing1 = seg_hashtable_putifabsent(table, key1, length1, value1);
  CU_ASSERT_PTR_EQUAL(existing1, value1);
  CU_ASSERT_EQUAL(seg_hashtable_count(table), 2);

  /* Retrieve an existing item. */
  void *existing0 = seg_hashtable_putifabsent(table, key0, length0, value01);
  CU_ASSERT_PTR_EQUAL(existing0, value0);
  CU_ASSERT_EQUAL(seg_hashtable_count(table), 2);

  void *out0 = seg_hashtable_get(table, key0, length0);
  CU_ASSERT_PTR_EQUAL(out0, value0);

  void *out1 = seg_hashtable_get(table, key1, length1);
  CU_ASSERT_PTR_EQUAL(out1, value1);

  seg_delete_hashtable(table);
}

typedef struct {
  int correct;
  int incorrect;
} iterator_state;

void hashtable_iterator(const char *key, const size_t key_length, const void *value, void *state)
{
  iterator_state *s = (iterator_state *) state;

  if (! memcmp(key, "aaa", 3) && ! memcmp(value, "aval", 4)) {
    s->correct++;
  } else if (! memcmp(key, "bbb", 3) && ! memcmp(value, "bval", 4)) {
    s->correct++;
  } else if (! memcmp(key, "ccc", 3) && ! memcmp(value, "cval", 4)) {
    s->correct++;
  } else if (! memcmp(key, "ddd", 3) && ! memcmp(value, "dval", 4)) {
    s->correct++;
  } else {
    printf("Unexpected pair in hash! [%.*s] -> [%s]\n", (int) key_length, key, (const char*) value);
    s->incorrect++;
  }
}

void test_hashtable_each(void)
{
  iterator_state s;
  s.correct = 0;
  s.incorrect = 0;

  seg_hashtablep table = seg_new_hashtable(3L);
  seg_hashtable_put(table, "aaa", 3, "aval");
  seg_hashtable_put(table, "bbb", 3, "bval");
  seg_hashtable_put(table, "ccc", 3, "cval");
  seg_hashtable_put(table, "ddd", 3, "dval");
  CU_ASSERT_EQUAL(seg_hashtable_count(table), 4);

  seg_hashtable_each(table, &hashtable_iterator, &s);

  CU_ASSERT_EQUAL(s.correct, 4);
  CU_ASSERT_EQUAL(s.incorrect, 0);

  seg_delete_hashtable(table);
}

CU_pSuite initialize_hashtable_suite(void)
{
  CU_pSuite pSuite = CU_add_suite("hashtable", NULL, NULL);
  if (pSuite == NULL) {
    return NULL;
  }

  ADD_TEST(test_hashtable_access);
  ADD_TEST(test_hashtable_putifabsent);
  ADD_TEST(test_hashtable_each);

  return pSuite;
}
