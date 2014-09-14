#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "model/object.h"
#include "runtime/runtime.h"
#include "runtime/symboltable.h"

struct seg_object {
  seg_object *class;
};

seg_object *seg_class(seg_object *object, seg_runtime *r) {
  return NULL;
}

// SEG_INTEGER /////////////////////////////////////////////////////////////////////////////////////

seg_err seg_integer(int64_t value, seg_object **out) {
  return SEG_NOTYET;
}

seg_err seg_integer_value(seg_object *object, int64_t *out) {
  return SEG_NOTYET;
}

// SEG_STRING //////////////////////////////////////////////////////////////////////////////////////

seg_err seg_string(char *str, uint64_t length, seg_object **out) {
  return SEG_NOTYET;
}

// SEG_SYMBOL //////////////////////////////////////////////////////////////////////////////////////

seg_err seg_symbol(char *str, uint64_t length, seg_object **out) {
  return SEG_NOTYET;
}

// SEG_SLOTTED /////////////////////////////////////////////////////////////////////////////////////

seg_err seg_slotted(seg_object *class, seg_object **out) {
  return SEG_NOTYET;
}

seg_err seg_slotted_capacity(seg_object *instance, uint64_t *out) {
  return SEG_NOTYET;
}

seg_err seg_slotted_length(seg_object *instance, uint64_t *out) {
  return SEG_NOTYET;
}

seg_err seg_slotted_setlength(seg_object *instance, uint64_t length) {
  return SEG_NOTYET;
}

seg_err seg_slotted_grow(seg_object *instance, uint64_t capacity) {
  return SEG_NOTYET;
}

seg_err seg_ivar_at(seg_object *o, uint64_t index, seg_object **out) {
  return SEG_NOTYET;
}

seg_err seg_ivar_atput(seg_object *o, uint64_t index, seg_object *v) {
  return SEG_NOTYET;
}

// BOOTSTRAPPING ///////////////////////////////////////////////////////////////////////////////////

seg_err _seg_bootstrap_runtime(seg_runtime *runtime, seg_bootstrap_objects *bootstrap) {
  return SEG_NOTYET;
}
