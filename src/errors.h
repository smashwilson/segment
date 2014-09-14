#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>

typedef enum {

  /* The operation completed successfully. */
  SEG_OK = 0,

  /* Attempt to allocated the required amount of memory failed. */
  SEG_NOMEM,

  /* Input parameter was outside of the valid range. */
  SEG_RANGE,

  /* Operation performed on an instance of an incorrect type. */
  SEG_TYPE,

  /* Operation not supported yet. */
  SEG_NOTYET

} seg_err;

#endif
