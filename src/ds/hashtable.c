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

/* Internal utility methods. */

void find_or_create_entry(
  seg_hashtablep table,
  const char *key,
  size_t key_length,
  entry **ent,
  int *created
) {
  uint32_t hashcode = murmur3_32(key, (uint32_t) key_length, table->seed);
  uint32_t bnum = hashcode % table->capacity;

  bucket *buck = &(table->buckets[bnum]);
  entry *e = NULL;
  int bindex = 0;

  if (buck->content == NULL) {
    /* Create an empty bucket and return its first slot. */
    buck->capacity = 4;
    buck->length = 0;
    buck->content = calloc(buck->capacity, sizeof(entry));

    e = &(buck->content[0]);
  } else {
    /* Search for an item already present with this key. */
    for (int i = 0; i < buck->length; i++) {
      e = &(buck->content[i]);

      if (
        e->hashcode == hashcode && e->key_length == key_length &&
        ! memcmp(e->key, key, key_length)
      ) {
        /* Found! Return this bucket and mark it as existing. */
        *ent = e;
        *created = 0;
        return ;
      }
    }

    /* Append and return a new entry at the bucket's end. */
    bindex = buck->length;

    if (buck->length >= buck->capacity) {
      /* Expand an existing bucket that has filled. */
      buck->capacity = buck->capacity * 2;
      buck->content = realloc(buck->content, buck->capacity);
      memset(buck->content, 0, sizeof(entry) * buck->capacity);
    }

    e = &(buck->content[bindex]);
  }

  /* We either created or extended a bucket. Initialize the new entry. */
  buck->length++;

  e->hashcode = hashcode;
  e->key = key;
  e->key_length = key_length;

  *ent = e;
  *created = 1;
}

/* Public API. */

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
  entry *ent;
  int created;
  void *result = NULL;

  find_or_create_entry(table, key, key_length, &ent, &created);

  if (! created) {
    result = ent->value;
  }
  ent->value = value;

  return result;
}

void *seg_hashtable_putifabsent(
  seg_hashtablep table,
  const char *key,
  size_t key_length,
  void *value
) {
  entry *ent;
  int created;

  find_or_create_entry(table, key, key_length, &ent, &created);

  if (! created) {
    return ent->value;
  } else {
    ent->value = value;
    return value;
  }
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
  for (int b = 0; b < table->capacity; b++) {
    bucket *buck = &(table->buckets[b]);

    if (buck->content != NULL) {
      for (int e = 0; e < buck->length; e++) {
        entry *ent = &(buck->content[e]);

        (*iter)(ent->key, ent->key_length, ent->value, state);
      }
    }
  }
}

void seg_delete_hashtable(seg_hashtablep table)
{
  free(table->buckets);
  free(table);
}
