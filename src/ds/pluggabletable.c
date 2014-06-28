#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ds/pluggabletable.h"
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

struct seg_pluggabletable {
  uint64_t count;
  uint64_t capacity;
  seg_pluggabletable_equal equalf;
  seg_pluggabletable_hash hashf;
  seg_hashtable_settings settings;
  pg_bucket *buckets;
};

typedef struct {
  seg_pluggabletable *table;
  pg_bucket *nbuckets;
  size_t ncapacity;
} pg_resize_state;

/* Internal utility methods. */

void pg_find_or_create_entry(
  seg_pluggabletable *table,
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
        return ;
      }
    }

    /* Append and return a new entry at the bucket's end. */
    bindex = buck->length;

    if (buck->length >= buck->capacity) {
      /* Expand an existing bucket that has filled. */
      buck->capacity = buck->capacity * table->settings.bucket_growth_factor;
      buck->content = realloc(buck->content, buck->capacity);
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
}

/*
 * A new element has been added. Calculate the table's new load and trigger a capacity extension
 * if necessary.
 */
void pg_trigger_dynamic_resize(seg_pluggabletable *table)
{
  float load = table->count / (float) table->capacity;
  if (load >= table->settings.max_load) {
    seg_pluggabletable_resize(table, table->capacity * table->settings.table_growth_factor);
  }
}

void pg_resize_iter(const void *key, void *value, void *state)
{
  pg_resize_state *rs = (pg_resize_state*) state;
  pg_entry *e;
  bool created;

  /* Add this entry to the new buckets structure. */
  pg_find_or_create_entry(rs->table, rs->nbuckets, rs->ncapacity, key, &e, &created);

  if (! created) {
    fprintf(
      stderr,
      "segment: Unexpected collision during hash growth at key [%s]\n",
      (const char*) key
    );
  }

  e->value = value;
}

/* Public API. */

uint64_t seg_pluggabletable_count(seg_pluggabletable *table)
{
  return table->count;
}

uint64_t seg_pluggabletable_capacity(seg_pluggabletable *table)
{
  return table->capacity;
}

seg_hashtable_settings *seg_pluggabletable_get_settings(seg_pluggabletable *table)
{
  return &(table->settings);
}

seg_pluggabletable *seg_new_pluggabletable(
  uint64_t capacity,
  seg_pluggabletable_equal equalfunc,
  seg_pluggabletable_hash hashfunc
) {
  seg_pluggabletable *table = malloc(sizeof(struct seg_pluggabletable));
  table->capacity = capacity;
  table->equalf = equalfunc;
  table->hashf = hashfunc;
  table->count = 0L;

  table->settings.init_bucket_capacity = SEG_HT_INIT_BUCKET_CAPACITY;
  table->settings.bucket_growth_factor = SEG_HT_BUCKET_GROWTH_FACTOR;
  table->settings.max_load = SEG_HT_MAX_LOAD;
  table->settings.table_growth_factor = SEG_HT_TABLE_GROWTH_FACTOR;

  pg_bucket *buckets = calloc(capacity, sizeof(pg_bucket));
  table->buckets = buckets;

  return table;
}

void seg_pluggabletable_resize(seg_pluggabletable *table, uint64_t capacity)
{
  uint64_t orig_cap = table->capacity;
  uint64_t orig_count = table->count;
  if (orig_cap == capacity) {
    return;
  }

  pg_bucket *nbuckets = calloc(capacity, sizeof(pg_bucket));

  pg_resize_state state;
  state.table = table;
  state.nbuckets = nbuckets;
  state.ncapacity = capacity;

  seg_pluggabletable_each(table, pg_resize_iter, &state);

  free(table->buckets);

  table->capacity = capacity;
  table->count = orig_count;
  table->buckets = nbuckets;
}

void *seg_pluggabletable_put(seg_pluggabletable *table, const void *key, void *value)
{
  pg_entry *ent;
  bool created;
  void *result = NULL;

  pg_find_or_create_entry(table, table->buckets, table->capacity, key, &ent, &created);

  if (! created) {
    result = ent->value;
  }

  ent->value = value;

  if (created) {
    pg_trigger_dynamic_resize(table);
  }

  return result;
}

void *seg_pluggabletable_putifabsent(seg_pluggabletable *table, const void *key, void *value) {
  pg_entry *ent;
  bool created;

  pg_find_or_create_entry(table, table->buckets, table->capacity, key, &ent, &created);

  if (! created) {
    return ent->value;
  } else {
    ent->value = value;
    pg_trigger_dynamic_resize(table);
    return value;
  }
}

void *seg_pluggabletable_get(seg_pluggabletable *table, const void *key)
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

void seg_pluggabletable_each(seg_pluggabletable *table, seg_pluggabletable_iterator iter, void *state)
{
  for (int b = 0; b < table->capacity; b++) {
    pg_bucket *buck = &(table->buckets[b]);

    if (buck->content != NULL) {
      for (int e = 0; e < buck->length; e++) {
        pg_entry *ent = &(buck->content[e]);

        (*iter)(ent->key, ent->value, state);
      }
    }
  }
}

void seg_delete_pluggabletable(seg_pluggabletable *table)
{
  free(table->buckets);
  free(table);
}
