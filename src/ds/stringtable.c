#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "stringtable.h"
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

seg_err st_find_or_create_entry(
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

    if (buck->content == NULL) {
      return SEG_NOMEM("Unable to allocate stringtable bucket.");
    }

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
        return SEG_OK;
      }
    }

    /* Append and return a new entry at the bucket's end. */
    bindex = buck->length;

    if (buck->length >= buck->capacity) {
      /* Expand an existing bucket that has filled. */
      buck->capacity = buck->capacity * table->settings.bucket_growth_factor;
      buck->content = realloc(buck->content, buck->capacity);

      if (buck->content == NULL) {
        return SEG_NOMEM("Unable to expand stringtable bucket.");
      }

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

  return SEG_OK;
}

/*
 * A new element has been added. Calculate the table's new load and trigger a capacity extension
 * if necessary.
 */
seg_err st_trigger_dynamic_resize(seg_stringtable *table)
{
  float load = table->count / (float) table->capacity;
  if (load >= table->settings.max_load) {
    return seg_stringtable_resize(table, table->capacity * table->settings.table_growth_factor);
  }
  return SEG_OK;
}

seg_err st_resize_iter(const char *key, const uint64_t key_length, void *value, void *state)
{
  seg_err err;

  st_resize_state *rs = (st_resize_state*) state;
  st_entry *e;
  bool created;

  /* Add this entry to the new buckets structure. */
  err = st_find_or_create_entry(rs->table, rs->nbuckets, rs->ncapacity, key, key_length, &e, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    return SEG_COLLISION("Unexpected collision when resizing stringtable.");
  }

  e->value = value;
  return SEG_OK;
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

seg_err seg_new_stringtable(uint64_t capacity, seg_stringtable **out)
{
  seg_stringtable *table = malloc(sizeof(struct seg_stringtable));
  if (table == NULL) {
    return SEG_NOMEM("Unable to allocate stringtable.");
  }

  table->capacity = capacity;
  table->count = 0L;
  table->seed = (uint32_t) ((intptr_t) table) % UINT32_MAX;

  table->settings.init_bucket_capacity = SEG_HT_INIT_BUCKET_CAPACITY;
  table->settings.bucket_growth_factor = SEG_HT_BUCKET_GROWTH_FACTOR;
  table->settings.max_load = SEG_HT_MAX_LOAD;
  table->settings.table_growth_factor = SEG_HT_TABLE_GROWTH_FACTOR;

  st_bucket *buckets = calloc(capacity, sizeof(st_bucket));

  if (buckets == NULL) {
    return SEG_NOMEM("Unable to allocate buckets for stringtable.");
  }

  table->buckets = buckets;
  *out = table;

  return SEG_OK;
}

seg_err seg_stringtable_resize(seg_stringtable *table, uint64_t capacity)
{
  seg_err err;

  uint64_t orig_cap = table->capacity;
  uint64_t orig_count = table->count;
  if (orig_cap == capacity) {
    return SEG_OK;
  }

  st_bucket *nbuckets = calloc(capacity, sizeof(st_bucket));
  if (nbuckets == NULL) {
    return SEG_NOMEM("Unable to allocate new buckets to resize stringtable.");
  }

  st_resize_state state;
  state.table = table;
  state.nbuckets = nbuckets;
  state.ncapacity = capacity;

  err = seg_stringtable_each(table, st_resize_iter, &state);
  if (err != SEG_OK) {
    return err;
  }

  free(table->buckets);

  table->capacity = capacity;
  table->count = orig_count;
  table->buckets = nbuckets;

  return SEG_OK;
}

seg_err seg_stringtable_put(seg_stringtable *table, const char *key, size_t key_length, void *value, void **out)
{
  seg_err err;

  st_entry *ent;
  bool created;
  void *result = NULL;

  err = st_find_or_create_entry(table, table->buckets, table->capacity, key, key_length, &ent, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    result = ent->value;
  }

  ent->value = value;

  if (created) {
    err = st_trigger_dynamic_resize(table);
    if (err != SEG_OK) {
      return err;
    }
  }

  *out = result;
  return SEG_OK;
}

seg_err seg_stringtable_putifabsent(
  seg_stringtable *table,
  const char *key,
  size_t key_length,
  void *value,
  void **out
) {
  seg_err err;
  st_entry *ent;
  bool created;

  err = st_find_or_create_entry(table, table->buckets, table->capacity, key, key_length, &ent, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    *out = ent->value;
  } else {
    ent->value = value;
    err = st_trigger_dynamic_resize(table);
    if (err != SEG_OK) {
      return err;
    }

    *out = value;
  }

  return SEG_OK;
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

seg_err seg_stringtable_each(seg_stringtable *table, seg_stringtable_iterator iter, void *state)
{
  seg_err err;

  for (int b = 0; b < table->capacity; b++) {
    st_bucket *buck = &(table->buckets[b]);

    if (buck->content != NULL) {
      for (int e = 0; e < buck->length; e++) {
        st_entry *ent = &(buck->content[e]);

        err = (*iter)(ent->key, ent->key_length, ent->value, state);
        if (err != SEG_OK) {
          return err;
        }
      }
    }
  }

  return SEG_OK;
}

void seg_delete_stringtable(seg_stringtable *table)
{
  free(table->buckets);
  free(table);
}
