#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "model/object.h"
#include "model/klass.h"
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

seg_err seg_slotted(seg_runtime *r, seg_object klass, seg_object *out)
{
  seg_err err;
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

  // Verify that klass is indeed a class that specifies slotted storage.
  if (SEG_IS_IMMEDIATE(klass) || !SEG_SAME(klass.pointer->klass, boots->class_class)) {
    return SEG_TYPE("Attempt to instantiate an invalid class.");
  }

  seg_object storage_slot;
  int64_t storage_value;
  SEG_TRY(seg_slot_at(klass, SEG_CLASS_SLOT_STORAGE, &storage_slot));
  SEG_TRY(seg_integer_value(storage_slot, &storage_value));

  if (storage_value != SEG_STORAGE_SLOTTED) {
    return SEG_TYPE("Attempt to instantiate a slotted instance from a non-slotted class.");
  }

  // Read the preferred initial length from the class.
  seg_object length_slot;
  int64_t length_value;
  SEG_TRY(seg_slot_at(klass, SEG_CLASS_SLOT_LENGTH, &length_slot));
  SEG_TRY(seg_integer_value(length_slot, &length_value));

  seg_object_slotted *result;
  SEG_TRY(_slotted_alloc(length_value, &result));
  _slotted_init_header(result, klass, length_value);
  _slotted_init_slots(r, result);

  out->pointer = (seg_object_common*) result;

  return SEG_OK;
}

seg_err seg_slotted_length(seg_object slotted, uint64_t *out)
{
  seg_object_slotted *casted = (seg_object_slotted*) slotted.pointer;
  *out = casted->length;
  return SEG_OK;
}

seg_err seg_slotted_grow(seg_object *slotted, uint64_t length)
{
  seg_err err;
  seg_object_slotted *casted = (seg_object_slotted*) slotted->pointer;

  if (casted->length >= length) {
    return SEG_OK;
  }

  seg_object_slotted *bigger;
  SEG_TRY(_slotted_alloc(length, &bigger));
  _slotted_init_header(bigger, casted->common.klass, length);
  memcpy(bigger->slots, casted->slots, (size_t) casted->length);

  slotted->pointer = (seg_object_common*) bigger;
  free(casted);

  return SEG_OK;
}

seg_err seg_slot_at(seg_object slotted, uint64_t index, seg_object *out)
{
  seg_object_slotted *casted = (seg_object_slotted*) slotted.pointer;

  if (casted->length >= index) {
    return SEG_RANGE("Attempt to access invalid slot index");
  }

  *out = casted->slots[index];

  return SEG_OK;
}

seg_err seg_slot_atput(seg_object slotted, uint64_t index, seg_object value)
{
  seg_object_slotted *casted = (seg_object_slotted*) slotted.pointer;

  if (casted->length >= index) {
    return SEG_RANGE("Attempt to mutate invalid slot index");
  }

  casted->slots[index] = value;

  return SEG_OK;
}

// BOOTSTRAPPING ///////////////////////////////////////////////////////////////////////////////////

seg_err _seg_bootstrap_runtime(seg_runtime *runtime, seg_bootstrap_objects *bootstrap)
{
  seg_err err;
  seg_symboltable *symtable = seg_runtime_symboltable(runtime);

  // Prepopulate the symbol table with symbols that we're going to use as names later.
  seg_object sym_name, sym_storage, sym_length, sym_ivars;
  SEG_TRY(seg_symboltable_cintern(symtable, "name", &sym_name));
  SEG_TRY(seg_symboltable_cintern(symtable, "storage", &sym_storage));
  SEG_TRY(seg_symboltable_cintern(symtable, "preferred_length", &sym_length));
  SEG_TRY(seg_symboltable_cintern(symtable, "instance_variables", &sym_ivars));

  seg_object sym_name_class;
  SEG_TRY(seg_symboltable_cintern(symtable, "Class", &sym_name_class));

  // Construct immediates that we'll need.
  seg_object slotted_storage, preferred_length;
  SEG_TRY(seg_integer(runtime, (int64_t) SEG_STORAGE_SLOTTED, &slotted_storage));
  SEG_TRY(seg_integer(runtime, (int64_t) SEG_CLASS_SLOTCOUNT, &preferred_length));

  // Initialize the Class class, which has itself as a class.
  // This is tricky because we can't use seg_slotted (or seg_class) to initialize Class itself, or
  // the sanity checks will fail.

  seg_object class_class;
  seg_object_slotted *class_class_internal;

  SEG_TRY(_slotted_alloc(SEG_CLASS_SLOTCOUNT, &class_class_internal));
  class_class.pointer = (seg_object_common*) class_class_internal;
  _slotted_init_header(class_class_internal, class_class, SEG_CLASS_SLOTCOUNT);

  SEG_TRY(seg_slot_atput(class_class, (uint64_t) SEG_CLASS_SLOT_NAME, sym_name_class));
  SEG_TRY(seg_slot_atput(class_class, (uint64_t) SEG_CLASS_SLOT_STORAGE, slotted_storage));
  SEG_TRY(seg_slot_atput(class_class, (uint64_t) SEG_CLASS_SLOT_LENGTH, preferred_length));

  bootstrap->class_class = class_class;

  // Instantiate the Array class and instance, then correct the ivars slots in the two classes created so far.
  SEG_TRY(seg_class(runtime, "Array", SEG_STORAGE_SLOTTED, &bootstrap->array_class));

  seg_object empty_array_0, empty_array_1;
  SEG_TRY(seg_slotted(runtime, bootstrap->array_class, &empty_array_0));
  SEG_TRY(seg_slotted(runtime, bootstrap->array_class, &empty_array_1));

  SEG_TRY(seg_slot_atput(bootstrap->class_class, (uint64_t) SEG_CLASS_SLOT_IVARS, empty_array_0));
  SEG_TRY(seg_slot_atput(bootstrap->array_class, (uint64_t) SEG_CLASS_SLOT_IVARS, empty_array_1));

  // Initialize the rest of the well-known class objects.
  SEG_TRY(seg_class(runtime, "Integer", SEG_STORAGE_IMMEDIATE, &bootstrap->integer_class));
  SEG_TRY(seg_class(runtime, "Float", SEG_STORAGE_IMMEDIATE, &bootstrap->float_class));
  SEG_TRY(seg_class(runtime, "String", SEG_STORAGE_BUFFER, &bootstrap->string_class));
  SEG_TRY(seg_class(runtime, "Symbol", SEG_STORAGE_BUFFER, &bootstrap->symbol_class));
  SEG_TRY(seg_class(runtime, "Array", SEG_STORAGE_SLOTTED, &bootstrap->array_class));
  SEG_TRY(seg_class(runtime, "Block", SEG_STORAGE_BUFFER, &bootstrap->block_class));

  return SEG_OK;
}
