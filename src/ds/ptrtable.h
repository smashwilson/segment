#ifndef PTRTABLE_H
#define PTRTABLE_H

#include <stdint.h>
#include <stddef.h>

#include "ds/hashtable.h"

/*
 * Hashtable specialized for keys that are directly comparable and of uniform length. This is
 * most useful for tables keyed by object identity, like symbols, or by instances of a common
 * struct.
 */
struct seg_ptrtable;
typedef struct seg_ptrtable seg_ptrtable;

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
