#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdint.h>

#include "errors.h"
#include "model/object.h"
#include "ds/hashtable.h"

struct seg_symboltable;
typedef struct seg_symboltable seg_symboltable;

/*
 * Signature of a function used to iterate over the key-value pairs within a hashtable.
 */
typedef seg_err (*seg_symboltable_iterator)(seg_object symbol, void *state);

/*
 * Default growth characteristics of the symbol table. Each of these may be overridden by the
 * environment variable of the same name, or controlled at runtime through the Symboltable object.
 */

#define SEG_SYMTABLE_CAP 1048576
#define SEG_SYMTABLE_GROWTH 2
#define SEG_SYMTABLE_BUCKET_CAP 4
#define SEG_SYMTABLE_BUCKET_GROWTH 2
#define SEG_SYMTABLE_MAX_LOAD 0.75

/*
 * Allocate a new symboltable for the interpreter. Read initial storage settings for the table from
 * the process' environment.
 *
 * SEG_NOMEM: If the allocation fails.
 */
seg_err seg_new_symboltable(seg_runtime *r, seg_symboltable **out);

/*
 * Insert a new entry into the symboltable if it's not already present. Return the newly created
 * symbol or the previously existing one.
 *
 * SEG_NOMEM: If the allocation of a new symbol fails.
 * SEG_RANGE: If the symbol length is greater than seg_symbol() permits.
 */
seg_err seg_symboltable_intern(seg_symboltable *table, const char *name, uint64_t length, seg_object *out);

/*
 * Convenience function to intern a symbol from a C-style NULL-terminated string.
 */
seg_err seg_symboltable_cintern(seg_symboltable *table, const char *name, seg_object *out);

/*
 * Access an existing symbol if one exists with the given name. Return SEG_NO_SYMBOL if it does not.
 */
seg_object seg_symboltable_get(seg_symboltable *table, const char *name, uint64_t length);

#define SEG_NO_SYMBOL SEG_NULL

/*
 * Return the number of non-immediate symbols currently stored in the symboltable.
 */
uint64_t seg_symboltable_count(seg_symboltable *table);

/*
 * Return the current maximum capacity of the symboltable.
 */
uint64_t seg_symboltable_capacity(seg_symboltable *table);

/*
* Retrieve the growth settings currently used by the symboltable. The settings are read-write.
*/
seg_hashtable_settings *seg_symboltable_get_settings(seg_symboltable *table);

/*
* Iterate through each interned symbol. `state` will be provided as-is to the
* iterator function during each iteration.
*/
seg_err seg_symboltable_each(seg_symboltable *table, seg_symboltable_iterator iter, void *state);

/*
 * Cleanly dispose of a symboltable allocated with `seg_new_symboltable`.
 */
void seg_delete_symboltable(seg_symboltable *table);

#endif
