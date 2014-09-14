#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>

// FIXME use seg_err for error reporting

/*
 * Settings that control the growth behavior of a hashtable.
 */
typedef struct {
  /* The initial capacity of newly allocated buckets. */
  uint32_t init_bucket_capacity;

  /* Factor by which bucket capacity will increase when filled. */
  uint32_t bucket_growth_factor;

  /* Load factor at which automatic resizing will be triggered. */
  float max_load;

  /* Amount by which the table's bucket capacity will grow when `max_load_factor` is reached. */
  uint32_t table_growth_factor;
} seg_hashtable_settings;

/* Default settings for a newly initialized table. */

#define SEG_HT_INIT_BUCKET_CAPACITY 4
#define SEG_HT_BUCKET_GROWTH_FACTOR 2
#define SEG_HT_MAX_LOAD 0.75
#define SEG_HT_TABLE_GROWTH_FACTOR 2

#endif
