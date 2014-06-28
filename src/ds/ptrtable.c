#include "ptrtable.h"

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

void pt_find_or_create_entry(
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
    buck->content = calloc(buck->capacity, sizeof(st_entry));

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

  *ent = e;
  *created = true;
}
