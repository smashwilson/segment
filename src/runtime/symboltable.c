#include <stdlib.h>
#include <string.h>

#include "runtime/symboltable.h"

seg_err seg_new_symboltable(seg_symboltable **out)
{
  char *end = NULL;

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

  seg_stringtable *table = seg_new_stringtable(capacity);

  seg_hashtable_settings *settings = seg_stringtable_get_settings(table);
  settings->init_bucket_capacity = init_bucket_capacity;
  settings->bucket_growth_factor = bucket_growth_factor;
  settings->max_load = max_load;
  settings->table_growth_factor = table_growth_factor;

  out = &table;
  return SEG_OK;
}

seg_err seg_symboltable_intern(seg_symboltable *table, const char *name, uint64_t length, seg_object **out)
{
  seg_err err;
  seg_object *existing = (seg_object *) seg_stringtable_get(table, name, length);

  if (existing != SEG_NO_SYMBOL) {
    out = &existing;
    return SEG_OK;
  }

  char *ownname = malloc(sizeof(char) * length);
  if (ownname == NULL) {
    return SEG_NOMEM;
  }
  ownname = memcpy(ownname, name, length);

  seg_object *created = NULL;
  err = seg_symbol(ownname, length, &created);
  if (err != SEG_OK) {
    return err;
  }
  seg_stringtable_put(table, name, length, created);

  out = &created;
  return SEG_OK;
}

seg_object *seg_symboltable_get(seg_symboltable *table, const char *name, size_t length)
{
  return (seg_object*) seg_stringtable_get(table, name, length);
}

void seg_delete_symboltable(seg_symboltable *table)
{
  seg_delete_stringtable(table);
}
