#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <stdarg.h>

#include "runtime/runtime.h"
#include "model/errors.h"
#include "model/object.h"

/*
 * Construct an empty Array instance with zero length and capacity.
 */
seg_err seg_empty_array(seg_runtime *r, seg_object *out);

/*
 * Construct an Array instance from a C array of seg_object references.
 */
seg_err seg_carray(
  seg_runtime *r,
  uint32_t capacity,
  uint32_t length,
  seg_object *contents,
  seg_object *out
);

/*
 * Construct an Array instance from a sequence of seg_object references using varargs.
 */
seg_err seg_varray(seg_runtime *r, uint32_t capacity, uint32_t length, seg_object *out, ...);

#endif /* end of include guard: ARRAY_H */
