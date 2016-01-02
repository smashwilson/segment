#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <stdarg.h>

#include "runtime/runtime.h"
#include "model/errors.h"
#include "model/object.h"

/*
 * Construct an empty Array instance with zero length and a specified initial capacity.
 */
seg_err seg_empty_array(seg_runtime *r, uint64_t capacity, seg_object *out);

/*
 * Construct an Array instance from a C array of seg_object references.
 */
seg_err seg_carray(
  seg_runtime *r,
  uint64_t capacity,
  uint64_t length,
  seg_object *contents,
  seg_object *out
);

/*
 * Construct an Array instance from a sequence of seg_object references using varargs.
 */
seg_err seg_varray(seg_runtime *r, uint64_t capacity, uint64_t length, seg_object *out, ...);

/*
 * Access the current maximum capacity of an Array.
 */
seg_err seg_array_capacity(seg_object array, uint64_t *capacity);

/*
 * Access the occupied length of an Array.
 */
seg_err seg_array_length(seg_object array, uint64_t *length);

#endif /* end of include guard: ARRAY_H */
