#include "model/array.h"

seg_err seg_empty_array(seg_runtime *r, seg_object *out)
{
  return SEG_NOTYET("seg_empty_array");
}

seg_err seg_carray(
  seg_runtime *r,
  uint32_t capacity,
  uint32_t length,
  seg_object *contents,
  seg_object *out
) {
  return SEG_NOTYET("seg_carray");
}

seg_err seg_varray(seg_runtime *r, uint32_t capacity, uint32_t length, seg_object *out, ...)
{
  return SEG_NOTYET("seg_varray");
}
