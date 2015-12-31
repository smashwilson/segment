#ifndef HASH
#define HASH

#include <stddef.h>
#include <stdint.h>

#include "errors.h"
#include "ds/hashtable.h"

/*
 * Hashtable specialized for keys that are directly comparable, variable-sized, contiguous chunks
 * of memory. This is most useful for tables keyed with strings, including internal tables like
 * the symbol table.
 */
struct seg_stringtable;
typedef struct seg_stringtable seg_stringtable;

/*
 * Signature of a function used to iterate over the key-value pairs within a hashtable.
 */
typedef seg_err (*seg_stringtable_iterator)(
  const char *key,
  const uint64_t key_length,
  void *value,
  void *state
);

/*
 * Allocate a new stringtable with the specified initial capacity.
 */
seg_err seg_new_stringtable(uint64_t capacity, seg_stringtable **out);

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
seg_err seg_stringtable_resize(seg_stringtable *table, uint64_t capacity);

/*
 * Add a new item to the stringtable, expanding it if necessary. Return the value previously
 * assigned to `key` if one was present. Otherwise, return `NULL`.
 */
seg_err seg_stringtable_put(
  seg_stringtable *table,
  const char *key,
  size_t key_length,
  void *value,
  void **out
);

/*
 * Add a new item to the stringtable if and only if `key` is currently unassigned. Return the
 * existing item mapped to `key` if there was one, or the newly assigned `value` otherwise.
 */
seg_err seg_stringtable_putifabsent(
  seg_stringtable *table,
  const char *key,
  size_t key_length,
  void *value,
  void **out
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
seg_err seg_stringtable_each(seg_stringtable *table, seg_stringtable_iterator iter, void *state);

/*
 * Destroy a stringtable created with `seg_new_stringtable`.
 */
void seg_delete_stringtable(seg_stringtable *table);

#endif
