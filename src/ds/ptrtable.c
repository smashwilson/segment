#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ptrtable.h"
#include "murmur.h"

typedef struct {
  uint32_t hashcode;
  const void *key;
  void *value;
} pt_entry;

typedef struct {
  size_t length;
  size_t capacity;
  pt_entry *content;
} pt_bucket;

struct seg_ptrtable {
  uint32_t seed;
  uint64_t count;
  uint64_t capacity;
  size_t key_length;
  seg_hashtable_settings settings;
  pt_bucket *buckets;
};

typedef struct {
  seg_ptrtable *table;
  pt_bucket *nbuckets;
  size_t ncapacity;
} pt_resize_state;

/* Internal utility methods. */

seg_err pt_find_or_create_entry(
  seg_ptrtable *table,
  pt_bucket *buckets,
  uint64_t capacity,
  const void *key,
  pt_entry **ent,
  bool *created
) {
  uint32_t hashcode = murmur3_32(key, (uint32_t) table->key_length, table->seed);
  uint32_t bnum = hashcode % capacity;

  pt_bucket *buck = &(buckets[bnum]);
  pt_entry *e = NULL;
  int bindex = 0;

  if (buck->content == NULL) {
    /* Create an empty bucket and return its first slot. */
    buck->capacity = table->settings.init_bucket_capacity;
    buck->length = 0;
    buck->content = calloc(buck->capacity, sizeof(pt_entry));

    if (buck->content == NULL) {
      return SEG_NOMEM("Unable to allocate ptrtable bucket.");
    }

    e = &(buck->content[0]);
  } else {
    /* Search for an item already present with this key. */
    for (int i = 0; i < buck->length; i++) {
      e = &(buck->content[i]);

      if (
        e->hashcode == hashcode && ! memcmp(e->key, key, table->key_length)
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
        return SEG_NOMEM("Unable to expand ptrtable bucket.");
      }

      memset(buck->content, 0, sizeof(pt_entry) * buck->capacity);
    }

    e = &(buck->content[bindex]);
  }

  /* We either created or extended a bucket. Initialize the new entry. */
  table->count++;
  buck->length++;

  e->hashcode = hashcode;
  e->key = key;

  *ent = e;
  *created = true;

  return SEG_OK;
}

/*
 * A new element has been added. Calculate the table's new load and trigger a capacity extension
 * if necessary.
 */
seg_err pt_trigger_dynamic_resize(seg_ptrtable *table)
{
  float load = table->count / (float) table->capacity;
  if (load >= table->settings.max_load) {
    return seg_ptrtable_resize(table, table->capacity * table->settings.table_growth_factor);
  }
  return SEG_OK;
}

seg_err pt_resize_iter(const void *key, void *value, void *state)
{
  seg_err err;

  pt_resize_state *rs = (pt_resize_state*) state;
  pt_entry *e;
  bool created;

  /* Add this entry to the new buckets structure. */
  err = pt_find_or_create_entry(rs->table, rs->nbuckets, rs->ncapacity, key, &e, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    return SEG_COLLISION("Collision during expansion of ptrtable.");
  }

  e->value = value;
  return SEG_OK;
}

/* Public API. */

uint64_t seg_ptrtable_count(seg_ptrtable *table)
{
  return table->count;
}

uint64_t seg_ptrtable_capacity(seg_ptrtable *table)
{
  return table->capacity;
}

seg_hashtable_settings *seg_ptrtable_get_settings(seg_ptrtable *table)
{
  return &(table->settings);
}

seg_err seg_new_ptrtable(uint64_t capacity, uint64_t key_length, seg_ptrtable **out)
{
  seg_ptrtable *table = malloc(sizeof(struct seg_ptrtable));
  if (table == NULL) {
    return SEG_NOMEM("Unable to allocate ptrtable.");
  }

  table->capacity = capacity;
  table->key_length = key_length;
  table->count = 0L;
  table->seed = (uint32_t) ((intptr_t) table) % UINT32_MAX;

  table->settings.init_bucket_capacity = SEG_HT_INIT_BUCKET_CAPACITY;
  table->settings.bucket_growth_factor = SEG_HT_BUCKET_GROWTH_FACTOR;
  table->settings.max_load = SEG_HT_MAX_LOAD;
  table->settings.table_growth_factor = SEG_HT_TABLE_GROWTH_FACTOR;

  pt_bucket *buckets = calloc(capacity, sizeof(pt_bucket));
  if (buckets == NULL) {
    return SEG_NOMEM("Unable to allocate ptrtable buckets.");
  }

  table->buckets = buckets;
  *out = table;
  return SEG_OK;
}

seg_err seg_ptrtable_resize(seg_ptrtable *table, uint64_t capacity)
{
  seg_err err;

  uint64_t orig_cap = table->capacity;
  uint64_t orig_count = table->count;
  if (orig_cap == capacity) {
    return SEG_OK;
  }

  pt_bucket *nbuckets = calloc(capacity, sizeof(pt_bucket));
  if (nbuckets == NULL) {
    return SEG_NOMEM("Unable to allocate resized ptrtable buckets.");
  }

  pt_resize_state state;
  state.table = table;
  state.nbuckets = nbuckets;
  state.ncapacity = capacity;

  err = seg_ptrtable_each(table, pt_resize_iter, &state);
  if (err != SEG_OK) {
    return err;
  }

  free(table->buckets);

  table->capacity = capacity;
  table->count = orig_count;
  table->buckets = nbuckets;
  return SEG_OK;
}

seg_err seg_ptrtable_put(seg_ptrtable *table, const void *key, void *value, void **out)
{
  seg_err err;

  pt_entry *ent;
  bool created;
  void *result = NULL;

  err = pt_find_or_create_entry(table, table->buckets, table->capacity, key, &ent, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    result = ent->value;
  }

  ent->value = value;

  if (created) {
    err = pt_trigger_dynamic_resize(table);
    if (err != SEG_OK) {
      return err;
    }
  }

  *out = result;
  return SEG_OK;
}

seg_err seg_ptrtable_putifabsent(seg_ptrtable *table, const void *key, void *value, void **out) {
  seg_err err;

  pt_entry *ent;
  bool created;

  err = pt_find_or_create_entry(table, table->buckets, table->capacity, key, &ent, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    *out = ent->value;
  } else {
    ent->value = value;
    err = pt_trigger_dynamic_resize(table);
    if (err != SEG_OK) {
      return err;
    }
    *out = value;
  }

  return SEG_OK;
}

void *seg_ptrtable_get(seg_ptrtable *table, const void *key)
{
  uint32_t hashcode = murmur3_32(key, (uint32_t) table->key_length, table->seed);
  uint32_t bnum = hashcode % table->capacity;

  pt_bucket *buck = &(table->buckets[bnum]);
  int bindex = 0;

  if(buck->content == NULL) {
    /* Bucket empty. */
    return NULL;
  }

  for (int i = 0; i < buck->length; i++) {
    pt_entry *ent = &(buck->content[i]);

    if (
      ent->hashcode == hashcode && ! memcmp(ent->key, key, table->key_length)
    ) {
      /* Found it! */
      return ent->value;
    }
  }

  /* Not present. */
  return NULL;
}

seg_err seg_ptrtable_each(seg_ptrtable *table, seg_ptrtable_iterator iter, void *state)
{
  seg_err err;

  for (int b = 0; b < table->capacity; b++) {
    pt_bucket *buck = &(table->buckets[b]);

    if (buck->content != NULL) {
      for (int e = 0; e < buck->length; e++) {
        pt_entry *ent = &(buck->content[e]);

        err = (*iter)(ent->key, ent->value, state);
        if (err != SEG_OK) {
          return err;
        }
      }
    }
  }

  return SEG_OK;
}

void seg_delete_ptrtable(seg_ptrtable *table)
{
  free(table->buckets);
  free(table);
}
