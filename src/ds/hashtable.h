#ifndef HASH
#define HASH

#include <stddef.h>

struct seg_hashtable;
typedef struct seg_hashtable *seg_hashtablep;

typedef struct {
  /* The initial capacity of newly allocated buckets. */
  size_t init_bucket_capacity;

  /* Factor by which bucket capacity will increase when filled. */
  size_t bucket_growth_factor;

  /* Load factor at which automatic resizing will be triggered. */
  float max_load;

  /* Amount by which the table's bucket capacity will grow when `max_load_factor` is reached. */
  size_t table_growth_factor;
} seg_hashtable_settings;

/* Default settings for a newly initialized table. */
#define SEG_HT_INIT_BUCKET_CAPACITY 4
#define SEG_HT_BUCKET_GROWTH_FACTOR 2
#define SEG_HT_MAX_LOAD 0.75
#define SEG_HT_TABLE_GROWTH_FACTOR 2

typedef void (*seg_hashtable_iterator)(
  const char *key,
  const size_t key_length,
  void *value,
  void *state
);

/*
 * Allocate a new hash table with the specified initial capacity.
 */
seg_hashtablep seg_new_hashtable(unsigned long capacity);

/*
 * Return the number of items currently stored in the table.
 */
unsigned long seg_hashtable_count(seg_hashtablep table);

/*
 * Return the current capacity of the hashtable.
 */
size_t seg_hashtable_capacity(seg_hashtablep table);

/*
 * Retrieve the growth settings currently used by a hashtable. The settings are read-write.
 */
seg_hashtable_settings *seg_hashtable_get_settings(seg_hashtablep table);

/*
 * Resize a hashtable's capacity. O(n). Invoked automatically during put operations if the table's
 * load increases beyond the threshold. Notice that `capacity` can be greater or less than the
 * current capacity.
 */
void seg_hashtable_resize(seg_hashtablep table, size_t capacity);

/*
 * Add a new item to the hashtable, expanding it if necessary. Return the value
 * previously assigned to `key` if one was present. Otherwise, return `NULL`.
 */
void *seg_hashtable_put(
  seg_hashtablep table,
  const char *key,
  size_t key_length,
  void *value
);

/*
 * Add a new item to the hashtable if and only if `key` is currently unassigned.
 * Return the existing item mapped to `key` if there was one, or the newly
 * assigned `value` otherwise.
 */
void *seg_hashtable_putifabsent(
  seg_hashtablep table,
  const char *key,
  size_t key_length,
  void *value
);

/*
 * Search for an existing value in the hashtable at `key`. Return the value or
 * `NULL` if it's not present.
 */
void *seg_hashtable_get(seg_hashtablep table, const char *key, size_t key_length);

/*
 * Iterate through each key-value pair in the hashtable. `state` will be provided as-is to the
 * iterator function during each iteration.
 */
void seg_hashtable_each(seg_hashtablep table, seg_hashtable_iterator iter, void *state);

/*
 * Destroy a hashtable created with `seg_new_hashtable`.
 */
void seg_delete_hashtable(seg_hashtablep table);

#endif
