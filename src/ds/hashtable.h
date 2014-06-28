#ifndef HASH
#define HASH

#include <stddef.h>
#include <stdint.h>

/*
 * Hashtable specialized for keys that are directly comparable, variable-sized, contiguous chunks
 * of memory. This is most useful for tables keyed with strings, including internal tables like
 * the symbol table.
 */
struct seg_stringtable;
typedef struct seg_stringtable seg_stringtable;

/*
 * Hashtable specialized for keys that are directly comparable and of uniform length. This is
 * most useful for tables keyed by object identity, like symbols, or by instances of a common
 * struct.
 */
struct seg_ptrtable;
typedef struct seg_ptrtable seg_ptrtable;

/*
 * Settings that control the growth behavior of a hashtable.
 */
typedef struct {
  /* The initial capacity of newly allocated buckets. */
  uint32_t init_bucket_capacity;

  /* Factor by which bucket capacity will increase when filled. */
  uint32_t bucket_growth_factor;

  /* Load factor at which automatic resizing will be triggered. */
  float max_load;

  /* Amount by which the table's bucket capacity will grow when `max_load_factor` is reached. */
  uint32_t table_growth_factor;
} seg_hashtable_settings;

/* Default settings for a newly initialized table. */

#define SEG_HT_INIT_BUCKET_CAPACITY 4
#define SEG_HT_BUCKET_GROWTH_FACTOR 2
#define SEG_HT_MAX_LOAD 0.75
#define SEG_HT_TABLE_GROWTH_FACTOR 2

// STRINGTABLE /////////////////////////////////////////////////////////////////////////////////////

/*
 * Signature of a function used to iterate over the key-value pairs within a hashtable.
 */
typedef void (*seg_stringtable_iterator)(
  const char *key,
  const size_t key_length,
  void *value,
  void *state
);

/*
 * Allocate a new stringtable with the specified initial capacity.
 */
seg_stringtable *seg_new_stringtable(uint64_t capacity);

/*
 * Return the number of items currently stored in a stringtable.
 */
uint64_t seg_stringtable_count(seg_stringtable *table);

/*
 * Return the current capacity of a stringtable.
 */
uint64_t seg_stringtable_capacity(seg_stringtable *table);

/*
 * Retrieve the growth settings currently used by a stringtable. The settings are read-write.
 */
seg_hashtable_settings *seg_stringtable_get_settings(seg_stringtable *table);

/*
 * Resize a stringtable's capacity. O(n). Invoked automatically during put operations if the table's
 * load increases beyond the threshold. Notice that `capacity` can be greater or less than the
 * current capacity.
 */
void seg_stringtable_resize(seg_stringtable *table, uint64_t capacity);

/*
 * Add a new item to the stringtable, expanding it if necessary. Return the value previously
 * assigned to `key` if one was present. Otherwise, return `NULL`.
 */
void *seg_stringtable_put(
  seg_stringtable *table,
  const char *key,
  size_t key_length,
  void *value
);

/*
 * Add a new item to the stringtable if and only if `key` is currently unassigned. Return the
 * existing item mapped to `key` if there was one, or the newly assigned `value` otherwise.
 */
void *seg_stringtable_putifabsent(
  seg_stringtable *table,
  const char *key,
  size_t key_length,
  void *value
);

/*
 * Search for an existing value in the hashtable at `key`. Return the value or
 * `NULL` if it's not present.
 */
void *seg_stringtable_get(seg_stringtable *table, const char *key, size_t key_length);

/*
 * Iterate through each key-value pair in the hashtable. `state` will be provided as-is to the
 * iterator function during each iteration.
 */
void seg_stringtable_each(seg_stringtable *table, seg_stringtable_iterator iter, void *state);

/*
 * Destroy a stringtable created with `seg_new_stringtable`.
 */
void seg_delete_stringtable(seg_stringtable *table);

// PTRTABLE ////////////////////////////////////////////////////////////////////////////////////////

/*
 * Signature of a function used to iterate over the key-value pairs within a ptrtable.
 */
typedef void (*seg_ptrtable_iterator)(const void *key, void *value, void *state);

/*
 * Allocate a new ptrtable with the specified initial capacity and key size.
 */
seg_ptrtable *seg_new_ptrtable(uint64_t capacity, size_t key_length);

/*
 * Return the number of items currently stored in a ptrtable.
 */
uint64_t seg_ptrtable_count(seg_ptrtable *table);

/*
 * Return the current capacity of a ptrtable.
 */
uint64_t seg_ptrtable_capacity(seg_ptrtable *table);

/*
 * Retrieve the growth settings currently used by a ptrtable. The settings are read-write.
 */
seg_hashtable_settings *seg_stringtable_get_settings(seg_ptrtable *table);

/*
 * Resize a ptrtable's capacity. O(n). Invoked automatically during put operations if the table's
 * load increases beyond the threshold. Notice that `capacity` can be greater or less than the
 * current capacity.
 */
void seg_ptrtable_resize(seg_ptrtable *table, uint64_t capacity);

/*
 * Add a new item to the ptrtable, expanding it if necessary. Return the value previously
 * assigned to `key` if one was present. Otherwise, return `NULL`.
 */
void *seg_ptrtable_put(seg_ptrtable *table, const void *key, void *value);

/*
 * Add a new item to the stringtable if and only if `key` is currently unassigned. Return the
 * existing item mapped to `key` if there was one, or the newly assigned `value` otherwise.
 */
void *seg_ptrtable_putifabsent(
  seg_ptrtable *table,
  const void *key,
  void *value
);

/*
 * Search for an existing value in the ptrtable at `key`. Return the value or `NULL` if it's not
 * present.
 */
void *seg_ptrtable_get(seg_ptrtable *table, const void *key);

/*
 * Iterate through each key-value pair in the ptrtable. `state` will be provided as-is to the
 * iterator function during each iteration.
 */
void seg_ptrtable_each(seg_ptrtable *table, seg_ptrtable_iterator iter, void *state);

/*
 * Destroy a ptrtable created with `seg_new_ptrtable`.
 */
void seg_delete_ptrtable(seg_ptrtable *table);

#endif
