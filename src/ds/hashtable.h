#ifndef HASH
#define HASH

struct seg_hashtable;
typedef struct seg_hashtable *seg_hashtablep;

typedef void (*seg_hashtable_iterator)(const char *key, const void *value, void *state);

/*
 * Allocate a new hash table with the specified initial capacity.
 */
seg_hashtablep seg_new_hashtable(unsigned long capacity);

/*
 * Add a new item to the hashtable, expanding it if necessary. Return the value
 * previously assigned to `key` if one was present. Otherwise, return `NULL`.
 */
void *seg_hashtable_put(seg_hashtablep table, const char *key, const void *value);

/*
 * Add a new item to the hashtable if and only if `key` is currently unassigned.
 * Return the existing item mapped to `key` if there was one, or the newly
 * assigned `value` otherwise.
 */
void *seg_hashtable_putifabsent(seg_hashtablep table, const char *key, const void *value);

/*
 * Search for an existing value in the hashtable at `key`. Return the value or
 * `NULL` if it's not present.
 */
void *seg_hashtable_get(seg_hashtablep table, const char *key);

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
