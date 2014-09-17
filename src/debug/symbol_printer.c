#include <stdio.h>

#include "debug/symbol_printer.h"

#include "ds/hashtable.h"

static seg_err print_symbol(const char *key, const uint64_t key_length, void *value, void *state)
{
  printf("symbol: [%*s]\n", (int) key_length, key);
  return SEG_OK;
}

void seg_print_symboltable(seg_symboltable *table)
{
  seg_stringtable_each(table, print_symbol, NULL);

  seg_hashtable_settings *settings = seg_stringtable_get_settings(table);
  uint64_t count = seg_stringtable_count(table);
  uint64_t capacity = seg_stringtable_capacity(table);

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
