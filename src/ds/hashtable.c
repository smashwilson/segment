#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
  uint64_t count;
  size_t capacity;
  seg_hashtable_settings *settings;
  bucket *buckets;
};

typedef struct {
  seg_hashtablep table;
  bucket *nbuckets;
  size_t ncapacity;
} resize_state;

/* Internal utility methods. */

void find_or_create_entry(
  seg_hashtablep table,
  bucket *buckets,
  size_t capacity,
  const char *key,
  size_t key_length,
  entry **ent,
  int *created
) {
  uint32_t hashcode = murmur3_32(key, (uint32_t) key_length, table->seed);
  uint32_t bnum = hashcode % capacity;

  bucket *buck = &(buckets[bnum]);
  entry *e = NULL;
  int bindex = 0;

  if (buck->content == NULL) {
    /* Create an empty bucket and return its first slot. */
    buck->capacity = table->settings->init_bucket_capacity;
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
      buck->capacity = buck->capacity * table->settings->bucket_growth_factor;
      buck->content = realloc(buck->content, buck->capacity);
      memset(buck->content, 0, sizeof(entry) * buck->capacity);
    }

    e = &(buck->content[bindex]);
  }

  /* We either created or extended a bucket. Initialize the new entry. */
  table->count++;
  buck->length++;

  e->hashcode = hashcode;
  e->key = key;
  e->key_length = key_length;

  *ent = e;
  *created = 1;
}

/*
 * A new element has been added. Calculate the table's new load and trigger a capacity extension
 * if necessary.
 */
void trigger_dynamic_resize(seg_hashtablep table)
{
  float load = table->count / (float) table->capacity;
  if (load >= table->settings->max_load) {
    seg_hashtable_resize(table, table->capacity * table->settings->table_growth_factor);
  }
}

void resize_iter(const char *key, const size_t key_length, void *value, void *state)
{
  resize_state *rs = (resize_state*) state;
  entry *e;
  int created;

  /* Add this entry to the new buckets structure. */
  find_or_create_entry(rs->table, rs->nbuckets, rs->ncapacity, key, key_length, &e, &created);

  if (! created) {
    fprintf(
      stderr,
      "segment: Unexpected collision during hash growth at key [%.*s]\n",
      (int) key_length,
      key
    );
  }

  e->value = value;
}

/* Public API. */

unsigned long seg_hashtable_count(seg_hashtablep table)
{
  return table->count;
}

size_t seg_hashtable_capacity(seg_hashtablep table)
{
  return table->capacity;
}

seg_hashtablep seg_new_hashtable(unsigned long capacity)
{
  seg_hashtablep table = malloc(sizeof(struct seg_hashtable));
  table->capacity = capacity;
  table->count = 0L;
  table->seed = (uint32_t) ((intptr_t) table) % UINT32_MAX;

  seg_hashtable_settings *settings = malloc(sizeof(seg_hashtable_settings));
  settings->init_bucket_capacity = SEG_HT_INIT_BUCKET_CAPACITY;
  settings->bucket_growth_factor = SEG_HT_BUCKET_GROWTH_FACTOR;
  settings->max_load = SEG_HT_MAX_LOAD;
  settings->table_growth_factor = SEG_HT_TABLE_GROWTH_FACTOR;
  table->settings = settings;

  bucket *buckets = calloc(capacity, sizeof(bucket));
  table->buckets = buckets;

  return table;
}

void seg_hashtable_resize(seg_hashtablep table, size_t capacity)
{
  size_t orig_cap = table->capacity;
  unsigned long orig_count = table->count;
  if (orig_cap == capacity) {
    return;
  }

  bucket *nbuckets = calloc(capacity, sizeof(bucket));

  resize_state state;
  state.table = table;
  state.nbuckets = nbuckets;
  state.ncapacity = capacity;

  seg_hashtable_each(table, resize_iter, &state);

  free(table->buckets);

  table->capacity = capacity;
  table->count = orig_count;
  table->buckets = nbuckets;
}

void *seg_hashtable_put(seg_hashtablep table, const char *key, size_t key_length, void *value)
{
  entry *ent;
  int created;
  void *result = NULL;

  find_or_create_entry(table, table->buckets, table->capacity, key, key_length, &ent, &created);

  if (! created) {
    result = ent->value;
  }

  ent->value = value;

  if (created) {
    trigger_dynamic_resize(table);
  }

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

  find_or_create_entry(table, table->buckets, table->capacity, key, key_length, &ent, &created);

  if (! created) {
    return ent->value;
  } else {
    ent->value = value;
    trigger_dynamic_resize(table);
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
  free(table->settings);
  free(table->buckets);
  free(table);
}
