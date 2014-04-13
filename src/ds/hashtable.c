#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "murmur.h"

typedef struct {
  uint32_t hashcode;
  const char *key;
  size_t key_length;
  void *value;
} entry;

typedef struct {
  size_t length;
  size_t capacity;
  entry *content;
} bucket;

struct seg_hashtable {
  uint32_t seed;
  unsigned long count;
  size_t capacity;
  bucket *buckets;
};

seg_hashtablep seg_new_hashtable(unsigned long capacity)
{
  seg_hashtablep table = malloc(sizeof(struct seg_hashtable));
  table->capacity = capacity;
  table->count = 0L;
  table->seed = (uint32_t) ((intptr_t) table) % UINT32_MAX;

  bucket *buckets = calloc(capacity, sizeof(bucket));
  table->buckets = buckets;

  return table;
}

void *seg_hashtable_put(seg_hashtablep table, const char *key, size_t key_length, void *value)
{
  uint32_t hashcode = murmur3_32(key, (uint32_t) key_length, table->seed);
  uint32_t bnum = hashcode % table->capacity;

  bucket *buck = &(table->buckets[bnum]);
  int bindex = 0;

  if (buck->content == NULL) {
    /* Create an empty bucket and store this item in its first slot. */
    buck->capacity = 4;
    buck->length = 0;
    buck->content = calloc(buck->capacity, sizeof(entry));

    bindex = 0;
  } else {
    /* Search for an item already present with this key. */
    for (int i = 0; i < buck->length; i++) {
      entry *ent = &(buck->content[i]);

      if (
        ent->hashcode == hashcode && ent->key_length == key_length &&
        ! memcmp(ent->key, key, key_length)
      ) {
        /* Found! Replace the existing key with the new one and return the old-> */
        void *v = ent->value;
        ent->value = value;
        return v;
      }
    }

    /* Append this key-value pair at the end. */
    bindex = buck->length;

    if (buck->length >= buck->capacity) {
      /* Expand an existing bucket that has filled. */
      buck->capacity = buck->capacity * 2;
      buck->content = realloc(buck->content, buck->capacity);
      memset(buck->content, 0, sizeof(entry) * buck->capacity);
    }
  }

  entry *ent = &(buck->content[bindex]);
  ent->key = key;
  ent->key_length = key_length;
  ent->hashcode = hashcode;
  ent->value = value;

  buck->length++;

  return NULL;
}

void *seg_hashtable_putifabsent(
  seg_hashtablep table,
  const char *key,
  size_t key_length,
  void *value
) {
  return NULL;
}

void *seg_hashtable_get(seg_hashtablep table, const char *key, size_t key_length)
{
  uint32_t hashcode = murmur3_32(key, (uint32_t) key_length, table->seed);
  uint32_t bnum = hashcode % table->capacity;

  bucket *buck = &(table->buckets[bnum]);
  int bindex = 0;

  if(buck->content == NULL) {
    /* Bucket empty. */
    return NULL;
  }

  for (int i = 0; i < buck->length; i++) {
    entry *ent = &(buck->content[i]);

    if (
      ent->hashcode == hashcode && ent->key_length == key_length &&
      ! memcmp(ent->key, key, key_length)
    ) {
      /* Found it! */
      return ent->value;
    }
  }

  /* Not present. */
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
