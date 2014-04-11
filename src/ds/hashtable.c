#include <stddef.h>

#include "hashtable.h"

seg_hashtablep seg_new_hashtable(long capacity)
{
  return NULL;
}

void *seg_hashtable_put(seg_hashtablep table, const char *key, const void *value)
{
  return NULL;
}

void *seg_hashtable_putifabsent(seg_hashtablep table, const char *key, const void *value)
{
  return NULL;
}

void *seg_hashtable_get(seg_hashtablep table, const char *key)
{
   return NULL;
}

void seg_hashtable_each(seg_hashtablep table, seg_hashtable_iterator iter, void *state)
{
  /* */
}

void seg_delete_hashtable(seg_hashtablep table)
{
  /* */
}
