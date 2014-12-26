#include <stdio.h>

#include "debug/symbol_printer.h"

static seg_err print_symbol(seg_object symbol, void *state)
{
  char *name;
  uint64_t length;
  seg_err err;

  err = seg_stringlike_contents(&symbol, &name, &length);
  if (err != SEG_OK) {
    return err;
  }

  printf("symbol: [%*s]\n", (int) length, name);
  return SEG_OK;
}

void seg_print_symboltable(seg_symboltable *table)
{
  seg_symboltable_each(table, print_symbol, NULL);

  seg_hashtable_settings *settings = seg_symboltable_get_settings(table);
  uint64_t count = seg_symboltable_count(table);
  uint64_t capacity = seg_symboltable_capacity(table);

  printf("\nsymbol table statistics:\n");
  printf(" count: %lu out of %lu\n", (unsigned long) count, (unsigned long) capacity);
  printf(" settings: initial bucket capacity = %lu\n",
    (unsigned long) settings->init_bucket_capacity);
  printf(" settings: bucket growth factor = %lu\n",
    (unsigned long) settings->bucket_growth_factor);
  printf(" settings: maximum load = %f\n", settings->max_load);
  printf(" settings: table growth factor = %lu\n",
    (unsigned long) settings->table_growth_factor);
}
