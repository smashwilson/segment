#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
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
} st_entry;

typedef struct {
  size_t length;
  size_t capacity;
  st_entry *content;
} st_bucket;

struct seg_stringtable {
  uint32_t seed;
  uint64_t count;
  uint64_t capacity;
  seg_hashtable_settings settings;
  st_bucket *buckets;
};

typedef struct {
  seg_stringtable *table;
  st_bucket *nbuckets;
  size_t ncapacity;
} st_resize_state;

/* Internal utility methods. */

void st_find_or_create_entry(
  seg_stringtable *table,
  st_bucket *buckets,
  size_t capacity,
  const char *key,
  size_t key_length,
  st_entry **ent,
  bool *created
) {
  uint32_t hashcode = murmur3_32(key, (uint32_t) key_length, table->seed);
  uint32_t bnum = hashcode % capacity;

  st_bucket *buck = &(buckets[bnum]);
  st_entry *e = NULL;
  int bindex = 0;

  if (buck->content == NULL) {
    /* Create an empty bucket and return its first slot. */
    buck->capacity = table->settings.init_bucket_capacity;
    buck->length = 0;
    buck->content = calloc(buck->capacity, sizeof(st_entry));

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
        *created = false;
        return ;
      }
    }

    /* Append and return a new entry at the bucket's end. */
    bindex = buck->length;

    if (buck->length >= buck->capacity) {
      /* Expand an existing bucket that has filled. */
      buck->capacity = buck->capacity * table->settings.bucket_growth_factor;
      buck->content = realloc(buck->content, buck->capacity);
      memset(buck->content, 0, sizeof(st_entry) * buck->capacity);
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
  *created = true;
}

/*
 * A new element has been added. Calculate the table's new load and trigger a capacity extension
 * if necessary.
 */
void st_trigger_dynamic_resize(seg_stringtable *table)
{
  float load = table->count / (float) table->capacity;
  if (load >= table->settings.max_load) {
    seg_stringtable_resize(table, table->capacity * table->settings.table_growth_factor);
  }
}

void st_resize_iter(const char *key, const size_t key_length, void *value, void *state)
{
  st_resize_state *rs = (st_resize_state*) state;
  st_entry *e;
  bool created;

  /* Add this entry to the new buckets structure. */
  st_find_or_create_entry(rs->table, rs->nbuckets, rs->ncapacity, key, key_length, &e, &created);

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

uint64_t seg_stringtable_count(seg_stringtable *table)
{
  return table->count;
}

uint64_t seg_stringtable_capacity(seg_stringtable *table)
{
  return table->capacity;
}

seg_hashtable_settings *seg_stringtable_get_settings(seg_stringtable *table)
{
  return &(table->settings);
}

seg_stringtable *seg_new_stringtable(uint64_t capacity)
{
  seg_stringtable *table = malloc(sizeof(struct seg_stringtable));
  table->capacity = capacity;
  table->count = 0L;
  table->seed = (uint32_t) ((intptr_t) table) % UINT32_MAX;

  table->settings.init_bucket_capacity = SEG_HT_INIT_BUCKET_CAPACITY;
  table->settings.bucket_growth_factor = SEG_HT_BUCKET_GROWTH_FACTOR;
  table->settings.max_load = SEG_HT_MAX_LOAD;
  table->settings.table_growth_factor = SEG_HT_TABLE_GROWTH_FACTOR;

  st_bucket *buckets = calloc(capacity, sizeof(st_bucket));
  table->buckets = buckets;

  return table;
}

void seg_stringtable_resize(seg_stringtable *table, uint64_t capacity)
{
  uint64_t orig_cap = table->capacity;
  uint64_t orig_count = table->count;
  if (orig_cap == capacity) {
    return;
  }

  st_bucket *nbuckets = calloc(capacity, sizeof(st_bucket));

  st_resize_state state;
  state.table = table;
  state.nbuckets = nbuckets;
  state.ncapacity = capacity;

  seg_stringtable_each(table, st_resize_iter, &state);

  free(table->buckets);

  table->capacity = capacity;
  table->count = orig_count;
  table->buckets = nbuckets;
}

void *seg_stringtable_put(seg_stringtable *table, const char *key, size_t key_length, void *value)
{
  st_entry *ent;
  bool created;
  void *result = NULL;

  st_find_or_create_entry(table, table->buckets, table->capacity, key, key_length, &ent, &created);

  if (! created) {
    result = ent->value;
  }

  ent->value = value;

  if (created) {
    st_trigger_dynamic_resize(table);
  }

  return result;
}

void *seg_stringtable_putifabsent(
  seg_stringtable *table,
  const char *key,
  size_t key_length,
  void *value
) {
  st_entry *ent;
  bool created;

  st_find_or_create_entry(table, table->buckets, table->capacity, key, key_length, &ent, &created);

  if (! created) {
    return ent->value;
  } else {
    ent->value = value;
    st_trigger_dynamic_resize(table);
    return value;
  }
}

void *seg_stringtable_get(seg_stringtable *table, const char *key, size_t key_length)
{
  uint32_t hashcode = murmur3_32(key, (uint32_t) key_length, table->seed);
  uint32_t bnum = hashcode % table->capacity;

  st_bucket *buck = &(table->buckets[bnum]);
  int bindex = 0;

  if(buck->content == NULL) {
    /* Bucket empty. */
    return NULL;
  }

  for (int i = 0; i < buck->length; i++) {
    st_entry *ent = &(buck->content[i]);

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

void seg_stringtable_each(seg_stringtable *table, seg_stringtable_iterator iter, void *state)
{
  for (int b = 0; b < table->capacity; b++) {
    st_bucket *buck = &(table->buckets[b]);

    if (buck->content != NULL) {
      for (int e = 0; e < buck->length; e++) {
        st_entry *ent = &(buck->content[e]);

        (*iter)(ent->key, ent->key_length, ent->value, state);
      }
    }
  }
}

void seg_delete_stringtable(seg_stringtable *table)
{
  free(table->buckets);
  free(table);
}
