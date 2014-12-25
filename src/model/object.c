#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "model/object.h"
#include "runtime/runtime.h"

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
 * Stringlike objects (to include strings of various encodings and symbols) store their content
 * as an opaque sequence of bytes.
 */
typedef struct {
  seg_object_common common;
  uint64_t length;
  char bytes[];
} seg_object_stringlike;

/*
 * Most instances are slotted objects. Slotted objects contain references to one or more other
 * objects, indexed by instance variable name or by numeric offset.
 */
typedef struct {
  seg_object object;
  uint64_t length;
  seg_object slots[];
} seg_object_slotted;

seg_object seg_class(seg_runtime *r, seg_object object)
{
  if (object.bits.immediate) {
    const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);

    switch (object.bits.kind) {
    case SEG_IMM_INTEGER:
      return boots->integer_class;
    case SEG_IMM_FLOAT:
      return boots->float_class;
    case SEG_IMM_STRING:
      return boots->string_class;
    case SEG_IMM_SYMBOL:
      return boots->symbol_class;
    default:
      /* Default to integers, for now. */
      return boots->integer_class;
    }
  }

  return object.pointer->klass;
}

const seg_object SEG_NULL = {
  .pointer = NULL
};

bool seg_object_same(seg_object a, seg_object b)
{
  return SEG_SAME(a, b);
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

// SEG_STRINGLIKE //////////////////////////////////////////////////////////////////////////////////

static seg_err _stringlike(seg_runtime *r, const char *str, uint64_t length, bool is_string, seg_object *out) {
  if (length > SEG_STR_IMMLEN) {
    // Allocate a non-immediate string object.
    seg_object_stringlike *s = malloc(sizeof(seg_object_stringlike) + length);
    if (s == NULL) {
      return SEG_NOMEM("Unable to allocate a stringlike object.");
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
  return _stringlike(r, str, length, true, out);
}

seg_err seg_cstring(seg_runtime *r, const char *str, seg_object *out)
{
  return seg_string(r, str, strlen(str), out);
}

seg_err seg_symbol(seg_runtime *r, const char *str, uint64_t length, seg_object *out)
{
  return _stringlike(r, str, length, false, out);
}

seg_err seg_stringlike_contents(seg_object stringlike, char **out, uint64_t *length)
{
  if (stringlike.bits.immediate) {
    if (stringlike.bits.kind != SEG_IMM_STRING && stringlike.bits.kind != SEG_IMM_SYMBOL) {
      return SEG_TYPE("Non-string or symbol provided to seg_stringlike_contents");
    }

    *length = stringlike.bits.length;
    *out = ((char*) &(stringlike.bits)) + (SEG_STR_IMMLEN - *length);

    return SEG_OK;
  }

  seg_object_stringlike *casted = (seg_object_stringlike *) stringlike.pointer;

  *length = casted->length;
  *out = casted->bytes;

  return SEG_OK;
}

// SEG_SLOTTED /////////////////////////////////////////////////////////////////////////////////////

seg_err seg_slotted(seg_runtime *r, seg_object klass, seg_object *out)
{
  return SEG_NOTYET("seg_slotted");
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

seg_err seg_slot_atput(seg_object slotted, uint64_t index, seg_object *out)
{
  return SEG_NOTYET("seg_slot_atput");
}

// BOOTSTRAPPING ///////////////////////////////////////////////////////////////////////////////////

seg_err _seg_bootstrap_runtime(seg_runtime *runtime, seg_bootstrap_objects *bootstrap)
{
  return SEG_OK;
}
