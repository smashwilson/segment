#include "model/klass.h"
#include "runtime/runtime.h"
#include "runtime/symboltable.h"

/*
 * Class objects use slotted storage with instance variables at fixed indices given by the
 * seg_class_slots enum.
 */

seg_err seg_class(seg_runtime *r, const char *name, seg_storage storage, seg_object *out)
{
  seg_err err;
  seg_symboltable *table = seg_runtime_symboltable(r);
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object o_name, o_storage, o_length;
  SEG_TRY(seg_symboltable_cintern(table, name, &o_name));
  SEG_TRY(seg_integer(r, (int64_t) storage, &o_storage));
  SEG_TRY(seg_integer(r, (int64_t) 0, &o_length));
  SEG_TRY(seg_slotted(r, boots->class_class, out));

  // Ensure that the class object has enough slots for its own instance variables, then populate
  // them.
  SEG_TRY(seg_slotted_grow(*out, (uint64_t) SEG_CLASS_SLOTCOUNT));

  SEG_TRY(seg_slot_atput(*out, (uint64_t) SEG_CLASS_SLOT_NAME, o_name));
  SEG_TRY(seg_slot_atput(*out, (uint64_t) SEG_CLASS_SLOT_STORAGE, o_storage));
  SEG_TRY(seg_slot_atput(*out, (uint64_t) SEG_CLASS_SLOT_LENGTH, o_length));
  SEG_TRY(seg_slot_atput(*out, (uint64_t) SEG_CLASS_SLOT_IVARS, boots->none_instance));

  return SEG_OK;
}

seg_err seg_class_ivars(seg_runtime *r, seg_object klass, int64_t count, ...)
{
  va_list args;
  char *ivarname;
  seg_err err;

  seg_symboltable *table = seg_runtime_symboltable(r);

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object ivar_array;
  SEG_TRY(seg_slotted(r, boots->array_class, &ivar_array));
  SEG_TRY(seg_slotted_grow(ivar_array, count));

  va_start(args, count);

  for (int i = 0; i < count; i++) {
    ivarname = va_arg(args, char *);

    seg_object ivarsym;
    SEG_TRY(seg_symboltable_cintern(table, ivarname, &ivarsym));
    if (err != SEG_OK) {
      va_end(args);
      return err;
    }

    SEG_TRY(seg_slot_atput(ivar_array, i, ivarsym));
    if (err != SEG_OK) {
      va_end(args);
      return err;
    }
  }

  va_end(args);

  seg_object slot_count;
  SEG_TRY(seg_integer(r, count, &slot_count));
  SEG_TRY(seg_slot_atput(klass, SEG_CLASS_SLOT_LENGTH, slot_count));
  SEG_TRY(seg_slot_atput(klass, SEG_CLASS_SLOT_IVARS, ivar_array));

  return SEG_OK;
}
