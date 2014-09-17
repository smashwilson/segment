#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ds/plugtable.h"
#include "ds/murmur.h"

typedef struct {
  uint32_t hashcode;
  const void *key;
  void *value;
} pg_entry;

typedef struct {
  size_t length;
  size_t capacity;
  pg_entry *content;
} pg_bucket;

struct seg_plugtable {
  uint64_t count;
  uint64_t capacity;
  seg_plugtable_equal equalf;
  seg_plugtable_hash hashf;
  seg_hashtable_settings settings;
  pg_bucket *buckets;
};

typedef struct {
  seg_plugtable *table;
  pg_bucket *nbuckets;
  size_t ncapacity;
} pg_resize_state;

/* Internal utility methods. */

seg_err pg_find_or_create_entry(
  seg_plugtable *table,
  pg_bucket *buckets,
  uint64_t capacity,
  const void *key,
  pg_entry **ent,
  bool *created
) {
  uint32_t hashcode = (*table->hashf)(key);
  uint32_t bnum = hashcode % capacity;

  pg_bucket *buck = &(buckets[bnum]);
  pg_entry *e = NULL;
  int bindex = 0;

  if (buck->content == NULL) {
    /* Create an empty bucket and return its first slot. */
    buck->capacity = table->settings.init_bucket_capacity;
    buck->length = 0;
    buck->content = calloc(buck->capacity, sizeof(pg_entry));
    if (buck->content == NULL) {
      return SEG_NOMEM("Unable to allocate plugtable bucket");
    }

    e = &(buck->content[0]);
  } else {
    /* Search for an item already present with this key. */
    for (int i = 0; i < buck->length; i++) {
      e = &(buck->content[i]);

      if (
        e->hashcode == hashcode && (*table->equalf)(e->key, key)
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
        return SEG_NOMEM("Unable to expand an existing plugtable bucket.");
      }

      memset(buck->content, 0, sizeof(pg_entry) * buck->capacity);
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
seg_err pg_trigger_dynamic_resize(seg_plugtable *table)
{
  float load = table->count / (float) table->capacity;
  if (load >= table->settings.max_load) {
    return seg_plugtable_resize(table, table->capacity * table->settings.table_growth_factor);
  }
  return SEG_OK;
}

seg_err pg_resize_iter(const void *key, void *value, void *state)
{
  pg_resize_state *rs = (pg_resize_state*) state;
  pg_entry *e;
  bool created;

  /* Add this entry to the new buckets structure. */
  pg_find_or_create_entry(rs->table, rs->nbuckets, rs->ncapacity, key, &e, &created);

  if (! created) {
    return SEG_COLLISION("Unexpected collision during hash growth");
  }

  e->value = value;
  return SEG_OK;
}

/* Public API. */

uint64_t seg_plugtable_count(seg_plugtable *table)
{
  return table->count;
}

uint64_t seg_plugtable_capacity(seg_plugtable *table)
{
  return table->capacity;
}

seg_hashtable_settings *seg_plugtable_get_settings(seg_plugtable *table)
{
  return &(table->settings);
}

seg_err seg_new_plugtable(
  uint64_t capacity,
  seg_plugtable_equal equalfunc,
  seg_plugtable_hash hashfunc,
  seg_plugtable **out
) {
  seg_plugtable *table = malloc(sizeof(struct seg_plugtable));
  if (table == NULL) {
    return SEG_NOMEM("Unable to allocate hashtable");
  }

  table->capacity = capacity;
  table->equalf = equalfunc;
  table->hashf = hashfunc;
  table->count = 0L;

  table->settings.init_bucket_capacity = SEG_HT_INIT_BUCKET_CAPACITY;
  table->settings.bucket_growth_factor = SEG_HT_BUCKET_GROWTH_FACTOR;
  table->settings.max_load = SEG_HT_MAX_LOAD;
  table->settings.table_growth_factor = SEG_HT_TABLE_GROWTH_FACTOR;

  pg_bucket *buckets = calloc(capacity, sizeof(pg_bucket));
  if (buckets == NULL) {
    return SEG_NOMEM("Unable to allocate buckets for hashtable");
  }
  table->buckets = buckets;

  *out = table;
  return SEG_OK;
}

seg_err seg_plugtable_resize(seg_plugtable *table, uint64_t capacity)
{
  seg_err err;

  uint64_t orig_cap = table->capacity;
  uint64_t orig_count = table->count;
  if (orig_cap == capacity) {
    return SEG_OK;
  }

  pg_bucket *nbuckets = calloc(capacity, sizeof(pg_bucket));

  pg_resize_state state;
  state.table = table;
  state.nbuckets = nbuckets;
  state.ncapacity = capacity;

  err = seg_plugtable_each(table, pg_resize_iter, &state);
  if (err != SEG_OK) {
    return err;
  }

  free(table->buckets);

  table->capacity = capacity;
  table->count = orig_count;
  table->buckets = nbuckets;

  return SEG_OK;
}

seg_err seg_plugtable_put(seg_plugtable *table, const void *key, void *value, void **out)
{
  seg_err err;

  pg_entry *ent;
  bool created;
  void *result = NULL;

  err = pg_find_or_create_entry(table, table->buckets, table->capacity, key, &ent, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    result = ent->value;
  }

  ent->value = value;

  if (created) {
    err = pg_trigger_dynamic_resize(table);
    if (err != SEG_OK) {
      return err;
    }
  }

  *out = result;
  return SEG_OK;
}

seg_err seg_plugtable_putifabsent(seg_plugtable *table, const void *key, void *value, void **out) {
  seg_err err;

  pg_entry *ent;
  bool created;

  err = pg_find_or_create_entry(table, table->buckets, table->capacity, key, &ent, &created);
  if (err != SEG_OK) {
    return err;
  }

  if (! created) {
    *out = ent->value;
  } else {
    ent->value = value;
    err = pg_trigger_dynamic_resize(table);
    if (err != SEG_OK) {
      return err;
    }
    *out = value;
  }
  return SEG_OK;
}

void *seg_plugtable_get(seg_plugtable *table, const void *key)
{
  uint32_t hashcode = (*table->hashf)(key);
  uint32_t bnum = hashcode % table->capacity;

  pg_bucket *buck = &(table->buckets[bnum]);
  int bindex = 0;

  if(buck->content == NULL) {
    /* Bucket empty. */
    return NULL;
  }

  for (int i = 0; i < buck->length; i++) {
    pg_entry *ent = &(buck->content[i]);

    if (
      ent->hashcode == hashcode && (*table->equalf)(ent->key, key)
    ) {
      /* Found it! */
      return ent->value;
    }
  }

  /* Not present. */
  return NULL;
}

seg_err seg_plugtable_each(seg_plugtable *table, seg_plugtable_iterator iter, void *state)
{
  seg_err err;

  for (int b = 0; b < table->capacity; b++) {
    pg_bucket *buck = &(table->buckets[b]);

    if (buck->content != NULL) {
      for (int e = 0; e < buck->length; e++) {
        pg_entry *ent = &(buck->content[e]);

        err = (*iter)(ent->key, ent->value, state);
        if (err != SEG_OK) {
          return err;
        }
      }
    }
  }

  return SEG_OK;
}

void seg_delete_plugtable(seg_plugtable *table)
{
  free(table->buckets);
  free(table);
}
