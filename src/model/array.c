#include "model/array.h"

seg_err seg_empty_array(seg_runtime *r, uint64_t capacity, seg_object *out)
{
  seg_err err;
  seg_object length;
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  SEG_TRY(seg_slotted_with_length(r, boots->array_class, capacity + 1, out));

  SEG_TRY(seg_integer(r, 0, &length));
  SEG_TRY(seg_slot_atput(*out, 0, length));

  return SEG_OK;
}

seg_err seg_carray(
  seg_runtime *r,
  uint64_t capacity,
  uint64_t length,
  seg_object *contents,
  seg_object *out
) {
  return SEG_NOTYET("seg_carray");
}

seg_err seg_varray(seg_runtime *r, uint64_t capacity, uint64_t length, seg_object *out, ...)
{
  return SEG_NOTYET("seg_varray");
}

seg_err seg_array_capacity(seg_object array, uint64_t *capacity)
{
  seg_err err;
  uint64_t slotted_length;

  SEG_TRY(seg_slotted_length(array, &slotted_length));
  *capacity = slotted_length - 1;

  return SEG_OK;
}

seg_err seg_array_length(seg_object array, uint64_t *length)
{
  seg_err err;
  seg_object o_length;
  int64_t signed_length;

  SEG_TRY(seg_slot_at(array, 0, &o_length));
  SEG_TRY(seg_integer_value(o_length, &signed_length));

  *length = (uint64_t) signed_length;

  return SEG_OK;
}
