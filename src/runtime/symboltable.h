#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdint.h>

#include "errors.h"
#include "ds/stringtable.h"
#include "model/object.h"

typedef seg_stringtable seg_symboltable;

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
seg_err seg_new_symboltable(seg_symboltable **out);

/*
 * Insert a new entry into the symboltable if it's not already present. Return the newly created
 * symbol or the previously existing one.
 *
 * SEG_NOMEM: If the allocation of a new symbol fails.
 * SEG_RANGE: If the symbol length is greater than seg_symbol() permits.
 */
seg_err seg_symboltable_intern(seg_symboltable *table, const char *name, uint64_t length, seg_object **out);

/*
 * Access an existing symbol if one exists with the given name. Return SEG_NO_SYMBOL if it does not.
 */
seg_object *seg_symboltable_get(seg_symboltable *table, const char *name, uint64_t length);

#define SEG_NO_SYMBOL NULL

/*
 * Cleanly dispose of a symboltable allocated with `seg_new_symboltable`.
 */
void seg_delete_symboltable(seg_symboltable *table);

#endif
