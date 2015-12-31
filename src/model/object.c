#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "model/object.h"
#include "runtime/runtime.h"
#include "runtime/symboltable.h"

typedef enum {
  SEG_IMM_INTEGER = 1,
  SEG_IMM_FLOAT = 2,
  SEG_IMM_STRING = 3,
  SEG_IMM_SYMBOL = 4
} seg_imm_kinds;

/*
 * Storage shared by all heap-allocated (non-immediate) seg_object values.
 */
struct seg_object_common {

  /*
  * All objects contain at least a pointer to the Class object that instantiated them. The Class
  * class containers a pointer to itself.
  */
  seg_object klass;

};

/*
 * Buffers (to include strings of various encodings and symbols) store their content as an opaque
 * sequence of bytes.
 */
typedef struct {
  seg_object_common common;
  uint64_t length;
  char bytes[];
} seg_object_buffer;

/*
 * Most instances are slotted objects. Slotted objects contain references to one or more other
 * objects, indexed by instance variable name or by numeric offset.
 */
typedef struct {
  seg_object_common common;
  uint64_t length;
  seg_object slots[];
} seg_object_slotted;

seg_err seg_object_class(seg_runtime *r, seg_object instance, seg_object *out)
{
  if (instance.bits.immediate) {
    const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

    switch (instance.bits.kind) {
    case SEG_IMM_INTEGER:
      *out = boots->integer_class;
      break;
    case SEG_IMM_FLOAT:
      *out = boots->float_class;
      break;
    case SEG_IMM_STRING:
      *out = boots->string_class;
      break;
    case SEG_IMM_SYMBOL:
      *out = boots->symbol_class;
      break;
    default:
      return SEG_INVAL("Invalid immediate kind.");
    }
    return SEG_OK;
  }

  out->pointer = instance.pointer->klass.pointer;

  return SEG_OK;
}

const seg_object SEG_NULL = {
  .pointer = NULL
};

bool seg_object_same(seg_object a, seg_object b)
{
  return SEG_SAME(a, b);
}

seg_err seg_object_storage(seg_object o, seg_storage *out)
{
  return SEG_NOTYET("seg_object_storage");
}

seg_object seg_object_frompointer(void *p)
{
  seg_object o;
  o.pointer = (seg_object_common*) p;
  return o;
}

// SEG_INTEGER /////////////////////////////////////////////////////////////////////////////////////

seg_err seg_integer(seg_runtime *r, int64_t value, seg_object *out)
{
  if (value <= SEG_INTEGER_MIN || value >= SEG_INTEGER_MAX) {
    return SEG_RANGE("Integer out of immediate range.");
  }

  out->bits.immediate = 1;
  out->bits.kind = SEG_IMM_INTEGER;
  out->bits.length = 0;
  out->bits.body = value;

  return SEG_OK;
}

seg_err seg_integer_value(seg_object object, int64_t *out)
{
  if (!object.bits.immediate || object.bits.kind != SEG_IMM_INTEGER) {
    return SEG_TYPE("Object was not an integer");
  }

  *out = object.bits.body;

  return SEG_OK;
}

// SEG_BUFFER //////////////////////////////////////////////////////////////////////////////////////

static seg_err _buffer(seg_runtime *r, const char *str, uint64_t length, bool is_string, seg_object *out) {
  if (length > SEG_STR_IMMLEN) {
    // Allocate a non-immediate string object.
    seg_object_buffer *s = malloc(sizeof(seg_object_buffer) + length);
    if (s == NULL) {
      return SEG_NOMEM("Unable to allocate a buffer.");
    }

    const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
    if (is_string) {
      s->common.klass = boots->string_class;
    } else {
      s->common.klass = boots->symbol_class;
    }
    s->length = length;
    memcpy(s->bytes, str, length);

    out->pointer = (seg_object_common*) s;

    return SEG_OK;
  }

  // Allocate an immediate string or symbol object.
  out->bits.immediate = 1;
  if (is_string) {
    out->bits.kind = SEG_IMM_STRING;
  } else {
    out->bits.kind = SEG_IMM_SYMBOL;
  }
  out->bits.length = length;

  // FIXME This is specific to the endianness of the processor.
  // For little-endian systems, we should pack bytes the other way as:
  // package += v << ((SEG_STR_IMMLEN - i - 1) * 8)
  unsigned long packed = 0;
  for (int i = 0; i < length; i++) {
    unsigned long v = (unsigned long) str[i];
    packed += v << (i * 8);
  }
  out->bits.body = packed;

  return SEG_OK;
}

seg_err seg_string(seg_runtime *r, const char *str, uint64_t length, seg_object *out)
{
  return _buffer(r, str, length, true, out);
}

seg_err seg_cstring(seg_runtime *r, const char *str, seg_object *out)
{
  return seg_string(r, str, strlen(str), out);
}

seg_err seg_symbol(seg_runtime *r, const char *str, uint64_t length, seg_object *out)
{
  return _buffer(r, str, length, false, out);
}

seg_err seg_buffer_contents(seg_object *buffer, char **out, uint64_t *length)
{
  if (buffer->bits.immediate) {
    if (buffer->bits.kind != SEG_IMM_STRING && buffer->bits.kind != SEG_IMM_SYMBOL) {
      return SEG_TYPE("Non-string or symbol provided to seg_buffer_contents");
    }

    *length = buffer->bits.length;
    // FIXME The +1 here is only necessary on big-endian systems.
    *out = (char*) buffer + 1;

    return SEG_OK;
  }

  seg_object_buffer *casted = (seg_object_buffer *) buffer->pointer;

  *length = casted->length;
  *out = casted->bytes;

  return SEG_OK;
}

// SEG_SLOTTED /////////////////////////////////////////////////////////////////////////////////////

seg_err seg_slotted(seg_runtime *r, seg_object klass, seg_object *out)
{
  seg_err err;
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  // Verify that klass is indeed a class that specifies slotted storage.
  if (SEG_IS_IMMEDIATE(klass) || !SEG_SAME(klass.pointer->klass, boots->class_class)) {
    return SEG_TYPE("Attempt to instantiate a non-class.");
  }

  seg_object storage_slot;
  int64_t storage_value;
  err = seg_slot_at(klass, SEG_CLASS_SLOT_STORAGE, &storage_slot);
  if (err != SEG_OK) {
    return err;
  }
  err = seg_integer_value(storage_slot, &storage_value);
  if (err != SEG_OK) {
    return err;
  }
  if (storage_value != SEG_STORAGE_SLOTTED) {
    return SEG_TYPE("Attempt to instantiate a slotted instance from a non-slotted class.");
  }

  // Read the preferred initial length from the class.
  seg_object length_slot;
  int64_t length_value;
  err = seg_slot_at(klass, SEG_CLASS_SLOT_LENGTH, &length_slot);
  if (err != SEG_OK) {
    return err;
  }
  err = seg_integer_value(length_slot, &length_value);
  if (err != SEG_OK) {
    return err;
  }

  seg_object_slotted *result;
  err = _slotted_alloc(length_value, &result);
  if (err != SEG_OK) {
    return err;
  }
  _slotted_init_header(result, klass, length_value);
  _slotted_init_slots(result, klass);

  out->pointer = (seg_object_common*) result;

  return SEG_NOTYET("seg_slotted");
}

static seg_err _slotted_alloc(uint64_t length, seg_object_slotted **out)
{
  *out = malloc(sizeof(seg_object_slotted) + (length * sizeof(seg_object)));
  if (*out == NULL) {
    return SEG_NOMEM("Unable to allocate a slotted object.");
  }
  return SEG_OK;
}

static void _slotted_init_header(seg_object_slotted *object, seg_object klass, uint64_t length)
{
  object->common.klass.pointer = klass.pointer;
  object->length = length;
}

static void _slotted_init_slots(seg_runtime *r, seg_object_slotted *object)
{
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  for (uint64_t i = 0; i < object->length; i++) {
    object->slots[i] = boots->none_instance;
  }
}

seg_err seg_slotted_length(seg_object instance, uint64_t *out)
{
  return SEG_NOTYET("seg_slotted_length");
}

seg_err seg_slotted_grow(seg_object instance, uint64_t length)
{
  return SEG_NOTYET("seg_slotted_grow");
}

seg_err seg_slot_at(seg_object slotted, uint64_t index, seg_object *out)
{
  return SEG_NOTYET("seg_slot_at");
}

seg_err seg_slot_atput(seg_object slotted, uint64_t index, seg_object value)
{
  return SEG_NOTYET("seg_slot_atput");
}

// CLASSES /////////////////////////////////////////////////////////////////////////////////////////

seg_err seg_class(seg_runtime *r, const char *name, seg_storage storage, seg_object *out)
{
  return SEG_NOTYET("seg_class");
}

seg_err seg_class_ivars(seg_runtime *r, seg_object klass, int64_t count, ...)
{
  va_list args;
  char *ivarname;
  seg_err err;

  seg_symboltable *table = seg_runtime_symboltable(r);

  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  seg_object ivar_array;
  err = seg_slotted(r, boots->array_class, &ivar_array);
  if (err != SEG_OK) {
    return err;
  }
  err = seg_slotted_grow(ivar_array, count);
  if (err != SEG_OK) {
    return err;
  }

  va_start(args, count);

  for (int i = 0; i < count; i++) {
    ivarname = va_arg(args, char *);

    seg_object ivarsym;
    err = seg_symboltable_cintern(table, ivarname, &ivarsym);
    if (err != SEG_OK) {
      va_end(args);
      return err;
    }

    err = seg_slot_atput(ivar_array, i, ivarsym);
    if (err != SEG_OK) {
      va_end(args);
      return err;
    }
  }

  va_end(args);

  seg_object slot_count;
  err = seg_integer(r, count, &slot_count);
  if (err != SEG_OK) {
    return err;
  }
  err = seg_slot_atput(klass, SEG_CLASS_SLOT_SLOTS, slot_count);
  if (err != SEG_OK) {
    return err;
  }
  err = seg_slot_atput(klass, SEG_CLASS_SLOT_IVARS, ivar_array);
  if (err != SEG_OK) {
    return err;
  }

  return SEG_OK;
}


// BOOTSTRAPPING ///////////////////////////////////////////////////////////////////////////////////

seg_err _seg_bootstrap_runtime(seg_runtime *runtime, seg_bootstrap_objects *bootstrap)
{
  seg_err err;

  seg_symboltable *symtable = seg_runtime_symboltable(runtime);

  seg_object sym_name, sym_storage, sym_ivars;
  err = seg_symboltable_cintern(symtable, "name", &sym_name);
  if (err != SEG_OK) {
    return err;
  }
  err = seg_symboltable_cintern(symtable, "storage", &sym_storage);
  if (err != SEG_OK) {
    return err;
  }
  err = seg_symboltable_cintern(symtable, "instance_variables", &sym_ivars);

  // Start with the Class class, which has itself as a class.
  seg_object name_class;
  err = seg_cstring(runtime, "Class", &name_class);
  if (err != SEG_OK) {
    return err;
  }

  return SEG_OK;
}
