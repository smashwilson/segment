#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <stdint.h>

#include "ds/hashtable.h"

typedef seg_hashtablep seg_symboltablep;

typedef struct {
  const char *name;
  size_t length;
} seg_symbol;

/*
 * Default growth characteristics of the symbol table. Each of these may be overridden by the
 * environment variable of the same name.
 */

#define SEG_SYMTABLE_CAP 1048576
#define SEG_SYMTABLE_GROWTH 2
#define SEG_SYMTABLE_BUCKET_CAP 4
#define SEG_SYMTABLE_BUCKET_GROWTH 2
#define SEG_SYMTABLE_MAX_LOAD 0.75

/*
 * Allocate a new symboltable for the interpreter. Read initial storage settings for the table from
 * the process' environment.
 */
seg_symboltablep seg_new_symboltable();

/*
 * Insert a new entry into the symboltable if it's not already present. Return the newly created
 * symbol or the previously existing one.
 */
seg_symbol *seg_symboltable_add(seg_symboltablep table, const char *name, size_t length);

/*
 * Return true if the symboltable contains a symbol with the specified name, or false otherwise.
 */
int seg_symboltable_has(seg_symboltablep table, const char *name, size_t length);

/*
 * Cleanly dispose of a symboltable allocated with `seg_new_symboltable`.
 */
void seg_delete_symboltable(seg_symboltablep table);

#endif
