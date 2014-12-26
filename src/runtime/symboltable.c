#include <stdlib.h>
#include <string.h>

#include "ds/stringtable.h"
#include "runtime/symboltable.h"

struct seg_symboltable {
  seg_runtime *runtime;
  seg_stringtable *storage;
};

seg_err seg_new_symboltable(seg_runtime *r, seg_symboltable **out)
{
  seg_err err;
  char *end = NULL;

  seg_symboltable *table = malloc(sizeof(seg_symboltable));
  if (table == NULL) {
    return SEG_NOMEM("Unable to allocate symboltable.");
  }

  uint64_t capacity = SEG_SYMTABLE_CAP;
  uint32_t init_bucket_capacity = SEG_SYMTABLE_BUCKET_CAP;
  uint32_t bucket_growth_factor = SEG_SYMTABLE_BUCKET_GROWTH;
  float max_load = SEG_SYMTABLE_MAX_LOAD;
  uint32_t table_growth_factor = SEG_SYMTABLE_GROWTH;

  const char *capacity_str = getenv("SEG_SYMTABLE_INIT_CAP");
  if (capacity_str != NULL) {
    capacity = strtoul(capacity_str, &end, 10);
  }

  const char *bucketcap_str = getenv("SEG_SYMTABLE_GROWTH");
  if (bucketcap_str != NULL) {
    init_bucket_capacity = (uint32_t) strtoul(bucketcap_str, &end, 10);
  }

  const char *bucketgrowth_str = getenv("SEG_SYMTABLE_BUCKET_GROWTH");
  if (bucketgrowth_str != NULL) {
    bucket_growth_factor = (uint32_t) strtoul(bucketgrowth_str, &end, 10);
  }

  const char *maxload_str = getenv("SEG_SYMTABLE_MAX_LOAD");
  if (maxload_str != NULL) {
    max_load = strtof(maxload_str, &end);
  }

  const char *growth_str = getenv("SEG_SYMTABLE_GROWTH");
  if (growth_str != NULL) {
    table_growth_factor = (uint32_t) strtoul(growth_str, &end, 10);
  }

  seg_stringtable *storage;
  err = seg_new_stringtable(capacity, &storage);
  if (err != SEG_OK) {
    return err;
  }

  seg_hashtable_settings *settings = seg_stringtable_get_settings(storage);
  settings->init_bucket_capacity = init_bucket_capacity;
  settings->bucket_growth_factor = bucket_growth_factor;
  settings->max_load = max_load;
  settings->table_growth_factor = table_growth_factor;

  table->runtime = r;
  table->storage = storage;

  *out = table;
  return SEG_OK;
}

seg_err seg_symboltable_intern(seg_symboltable *table, const char *name, uint64_t length, seg_object *out)
{
  seg_err err;
  seg_object created;

  // Immediate values don't need to be stored in the symbol table. You can already compare them
  // for equality by a single pointer comparison.
  if (SEG_STR_WILLBEIMM(length)) {
    err = seg_symbol(table->runtime, name, length, &created);
    if (err != SEG_OK) {
      return err;
    }

    *out = created;
    return SEG_OK;
  }

  seg_object existing = SEG_FROMPOINTER(seg_stringtable_get(table->storage, name, length));

  if (!SEG_SAME(existing, SEG_NO_SYMBOL)) {
    *out = existing;
    return SEG_OK;
  }

  err = seg_symbol(table->runtime, name, length, &created);
  if (err != SEG_OK) {
    return err;
  }

  void *prior;
  err = seg_stringtable_put(table->storage, name, length, SEG_TOPOINTER(created), &prior);
  if (err != SEG_OK) {
    return err;
  }

  *out = created;
  return SEG_OK;
}

seg_object seg_symboltable_get(seg_symboltable *table, const char *name, uint64_t length)
{
  // Always treat immediate symbols as though they're in the symboltable.
  if (SEG_STR_WILLBEIMM(length)) {
    seg_err err;
    seg_object created;

    err = seg_symbol(table->runtime, name, length, &created);
    if (err != SEG_OK) {
      return SEG_NO_SYMBOL;
    }

    return created;
  }

  seg_object o = SEG_FROMPOINTER(seg_stringtable_get(table->storage, name, length));
  return o;
}

uint64_t seg_symboltable_count(seg_symboltable *table)
{
  return seg_stringtable_count(table->storage);
}

uint64_t seg_symboltable_capacity(seg_symboltable *table)
{
  return seg_stringtable_capacity(table->storage);
}

seg_hashtable_settings *seg_symboltable_get_settings(seg_symboltable *table)
{
  return seg_stringtable_get_settings(table->storage);
}

typedef struct {
  void *outer_state;
  seg_symboltable_iterator iter;
} symboliter_state;

static seg_err symboliter(const char *key, const uint64_t key_length, void *value, void *state)
{
  symboliter_state *casted = (symboliter_state*) state;
  seg_object o = SEG_FROMPOINTER(value);

  return casted->iter(o, casted->outer_state);
}

seg_err seg_symboltable_each(seg_symboltable *table, seg_symboltable_iterator iter, void *state)
{
  symboliter_state symbolstate;
  symbolstate.outer_state = state;
  symbolstate.iter = iter;

  return seg_stringtable_each(table->storage, symboliter, &symbolstate);
}

void seg_delete_symboltable(seg_symboltable *table)
{
  seg_delete_stringtable(table->storage);
  free(table);
}
