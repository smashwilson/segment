#ifndef PLUGGABLETABLE_H
#define PLUGGABLETABLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ds/hashtable.h"

/*
 * Hashtable that uses custom functions to compute key hashes and equality. This will ultimately
 * back in-language Dictionary instances.
 */
struct seg_pluggabletable;
typedef struct seg_pluggabletable seg_pluggabletable;

/*
 * Signature of a function used to iterate over the key-value pairs within a ptrtable.
 */
typedef void (*seg_pluggabletable_iterator)(const void *key, void *value, void *state);

/*
 * Signature of a custom key equality function.
 */
typedef bool (*seg_pluggabletable_equal)(const void *left, const void *right);

/*
 * Signature of a custom key hash function.
 */
typedef uint32_t (*seg_pluggabletable_hash)(const void *key);

/*
 * Allocate a new ptrtable with the specified initial capacity and equality and hash functions.
 */
seg_pluggabletable *seg_new_pluggabletable(
  uint64_t capacity,
  seg_pluggabletable_equal equalfunc,
  seg_pluggabletable_hash hashfunc
);

/*
 * Return the number of items currently stored in a ptrtable.
 */
uint64_t seg_pluggabletable_count(seg_pluggabletable *table);

/*
 * Return the current capacity of a ptrtable.
 */
uint64_t seg_pluggabletable_capacity(seg_pluggabletable *table);

/*
 * Retrieve the growth settings currently used by a ptrtable. The settings are read-write.
 */
seg_hashtable_settings *seg_stringtable_get_settings(seg_pluggabletable *table);

/*
 * Resize a ptrtable's capacity. O(n). Invoked automatically during put operations if the table's
 * load increases beyond the threshold. Notice that `capacity` can be greater or less than the
 * current capacity.
 */
void seg_pluggabletable_resize(seg_pluggabletable *table, uint64_t capacity);

/*
 * Add a new item to the ptrtable, expanding it if necessary. Return the value previously
 * assigned to `key` if one was present. Otherwise, return `NULL`.
 */
void *seg_pluggabletable_put(seg_pluggabletable *table, const void *key, void *value);

/*
 * Add a new item to the stringtable if and only if `key` is currently unassigned. Return the
 * existing item mapped to `key` if there was one, or the newly assigned `value` otherwise.
 */
void *seg_pluggabletable_putifabsent(
  seg_pluggabletable *table,
  const void *key,
  void *value
);

/*
 * Search for an existing value in the ptrtable at `key`. Return the value or `NULL` if it's not
 * present.
 */
void *seg_pluggabletable_get(seg_pluggabletable *table, const void *key);

/*
 * Iterate through each key-value pair in the ptrtable. `state` will be provided as-is to the
 * iterator function during each iteration.
 */
void seg_pluggabletable_each(seg_pluggabletable *table, seg_pluggabletable_iterator iter, void *state);

/*
 * Destroy a ptrtable created with `seg_new_pluggabletable`.
 */
void seg_delete_pluggabletable(seg_pluggabletable *table);

#endif
