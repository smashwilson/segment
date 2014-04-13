#include <stdio.h>

#include "symbol_printer.h"

#include "../ds/hashtable.h"

static void print_symbol(const char *key, size_t key_length, void *value, void *state)
{
  printf("symbol: [%*s]\n", (int) key_length, key);
}

void seg_print_symboltable(seg_symboltablep table)
{
  seg_hashtable_each(table, print_symbol, NULL);

  seg_hashtable_settings *settings = seg_hashtable_get_settings(table);
  unsigned long count = seg_hashtable_count(table);
  size_t capacity = seg_hashtable_capacity(table);

  printf("\nsymbol table statistics:\n");
  printf(" count: %lu out of %lu\n", count, (unsigned long) capacity);
  printf(" settings: initial bucket capacity = %lu\n", settings->init_bucket_capacity);
  printf(" settings: bucket growth factor = %lu\n", settings->bucket_growth_factor);
  printf(" settings: maximum load = %f\n", settings->max_load);
  printf(" settings: table growth factor = %lu\n", settings->table_growth_factor);
}
