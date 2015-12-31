#ifndef KLASS_H
#define KLASS_H

#include <stdint.h>
#include <stdarg.h>

#include "object.h"
#include "errors.h"

/* Slot indices used by Class objects. */
typedef enum {
  SEG_CLASS_SLOT_NAME = 0,
  SEG_CLASS_SLOT_STORAGE,
  SEG_CLASS_SLOT_LENGTH,
  SEG_CLASS_SLOT_IVARS,
  SEG_CLASS_SLOTCOUNT
} seg_class_slots;

/*
 * Instantiate a new class object.
 *
 * SEG_INVAL: if storage is invalid.
 */
seg_err seg_class(seg_runtime *r, const char *name, seg_storage storage, seg_object *out);

/*
 * Set a slotted class' instance variables to an Array of the specified names.
 *
 * SEG_INVAL: if klass doesn't have slotted storage.
 */
seg_err seg_class_ivars(seg_runtime *r, seg_object klass, int64_t count, ...);

/*
 * Access a class object's storage implementation.
 */
seg_err seg_class_storage(seg_object klass, seg_storage *out);

#endif
