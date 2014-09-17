#ifndef ERRORS_H
#define ERRORS_H

#include <stdint.h>

typedef enum {

  /* Attempt to allocated the required amount of memory failed. */
  SEG_CODE_NOMEM,

  /* Input parameter was outside of the valid range. */
  SEG_CODE_RANGE,

  /* Operation performed on an instance of an incorrect type. */
  SEG_CODE_TYPE,

  /* Operation not supported yet. */
  SEG_CODE_NOTYET

} seg_err_code;

typedef struct __seg_err {
  seg_err_code code;
  const char *message;
} *seg_err;

#define SEG_OK NULL

static struct __seg_err __seg_err_nomem = {
  .code = SEG_CODE_NOMEM,
  .message = "Out of memory"
};

#define __SEG_ERR(ecode, ebody) \
  do { \
    seg_err __seg_err_ ## __LINE__ = malloc(sizeof(seg_err)); \
    if __seg_err_ ## __LINE__ == NULL { \
      __seg_err_ ## __LINE__.code = (ecode); \
      __seg_err_ ## __LINE__.message = "__FILE__@L__LINE__:" ## (ebody); \
    } else { \
      &__seg_err_nomem \
    } \
  } while(0)

#define SEG_NOMEM(msg) __SEG_ERR(SEG_CODE_NOMEM, msg)
#define SEG_RANGE(msg) __SEG_ERR(SEG_CODE_RANGE, msg)
#define SEG_TYPE(msg) __SEG_ERR(SEG_CODE_TYPE, msg)
#define SEG_NOTYET(msg) __SEG_ERR(SEG_CODE_NOTYET, msg)

#endif
