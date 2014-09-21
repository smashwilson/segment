#ifndef ERRORS_H
#define ERRORS_H

#include <stdlib.h>

typedef enum {

  /* Attempt to allocated the required amount of memory failed. */
  SEG_CODE_NOMEM,

  /* Input parameter was outside of the valid range. */
  SEG_CODE_RANGE,

  /* Operation performed on an instance of an incorrect type. */
  SEG_CODE_TYPE,

  /* Unexpected hash collision encountered. */
  SEG_CODE_COLLISION,

  /* Operation not supported yet. */
  SEG_CODE_NOTYET

} seg_err_code;

typedef struct __seg_err {
  seg_err_code code;
  const char *message;
} *seg_err;

#define SEG_OK NULL

seg_err __seg_create_err(seg_err_code code, const char *msg);

#define __STRINGIZE_DETAIL(x) #x
#define __STRINGIZE(x) __STRINGIZE_DETAIL(x)

#define __PREFIX(msg) (__FILE__ "@L" __STRINGIZE(__LINE__) ": " msg)

#define SEG_NOMEM(msg) __seg_create_err(SEG_CODE_NOMEM, __PREFIX(msg))
#define SEG_RANGE(msg) __seg_create_err(SEG_CODE_RANGE, __PREFIX(msg))
#define SEG_TYPE(msg) __seg_create_err(SEG_CODE_TYPE, __PREFIX(msg))
#define SEG_COLLISION(msg) __seg_create_err(SEG_CODE_COLLISION, __PREFIX(msg))
#define SEG_NOTYET(msg) __seg_create_err(SEG_CODE_NOTYET, __PREFIX(msg))

#endif
