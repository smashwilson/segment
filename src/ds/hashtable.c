#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

typedef struct {
  size_t length;
  union {
    uint32_t hashcode;
    void *data;
  } *content;
} bucket;

struct seg_hashtable {
  unsigned long count;
  size_t capacity;
  bucket *buckets;
};

seg_hashtablep seg_new_hashtable(unsigned long capacity)
{
  seg_hashtablep table = malloc(sizeof(struct seg_hashtable));
  table->capacity = capacity;
  table->count = 0L;

  bucket *buckets = malloc(sizeof(bucket) * capacity);
  memset(buckets, 0, capacity);
  table->buckets = buckets;

  return table;
}

void *seg_hashtable_put(seg_hashtablep table, const char *key, const void *value)
{
  return NULL;
}

void *seg_hashtable_putifabsent(seg_hashtablep table, const char *key, const void *value)
{
  return NULL;
}

void *seg_hashtable_get(seg_hashtablep table, const char *key)
{
   return NULL;
}

void seg_hashtable_each(seg_hashtablep table, seg_hashtable_iterator iter, void *state)
{
  /* */
}

void seg_delete_hashtable(seg_hashtablep table)
{
  free(table->buckets);
  free(table);
}
