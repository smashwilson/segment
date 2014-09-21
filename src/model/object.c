#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "errors.h"
#include "model/object.h"
#include "runtime/runtime.h"

struct seg_object {
  uint64_t header;
};

struct seg_object_stringlike {
  seg_object object;
  char *bytes;
};

struct seg_object_slotted {
  seg_object object;
  uint64_t capacity;
  uint64_t length;
  seg_object **slots;
};

/*
 * Create a mask that occludes all but the topmost `width` bits of a uintptr_t, where
 * `width` is between 1 and 8.
 */
#define MASK(width) ((uint64_t) 0xFF << (64 - width))

/*
 * Create a uintptr_t whose top byte is the provided value, and all lower bits are 0x0.
 */
#define TOPBYTE(byte) ((uint64_t) (byte) << 56)

/* INTEGER header: [00]00 */
const uint64_t INTEGER_MASK = MASK(2);
const uint64_t INTEGER_FLAG = 0x0;

/* FLOAT header: [01]00 */
const uint64_t FLOAT_MASK = MASK(2);
const uint64_t FLOAT_FLAG = TOPBYTE(0x40);

/* SLOTTED header: [10]00 */
const uint64_t SLOTTED_MASK = MASK(2);
const uint64_t SLOTTED_FLAG = TOPBYTE(0x80);

/* STRING header: [110]0 */
const uint64_t STRING_MASK = MASK(3);
const uint64_t STRING_FLAG = TOPBYTE(0xc0);

/* SYMBOL header: [111]0 */
const uint64_t SYMBOL_MASK = MASK(3);
const uint64_t SYMBOL_FLAG = TOPBYTE(0xe0);

#define IS_INTEGER(object) (((object)->header & INTEGER_MASK) == INTEGER_FLAG)
#define IS_FLOAT(object) (((object)->header & FLOAT_MASK) == FLOAT_FLAG)
#define IS_SLOTTED(object) (((object)->header & SLOTTED_MASK) == SLOTTED_FLAG)
#define IS_STRING(object) (((object)->header & STRING_MASK) == STRING_FLAG)
#define IS_SYMBOL(object) (((object)->header & SYMBOL_MASK) == SYMBOL_FLAG)

seg_object *seg_class(seg_object *object, seg_runtime *r)
{
  const seg_bootstrap_objects *boots = seg_runtime_bootstraps(r);
  if (IS_INTEGER(object)) {
    return boots->integer_class;
  }
  if (IS_STRING(object)) {
    return boots->string_class;
  }
  if (IS_SYMBOL(object)) {
    return boots->symbol_class;
  }
  return NULL;
}

// SEG_INTEGER /////////////////////////////////////////////////////////////////////////////////////

struct imm_integer {
  unsigned top  :2;
  int64_t body  :62;
};

seg_err seg_integer(int64_t value, seg_object **out)
{
  seg_object *i = malloc(sizeof(seg_object));
  if (i == NULL) {
    return SEG_NOMEM("Unable to allocate integer object.");
  }
  if (value <= SEG_INTEGER_MIN || value >= SEG_INTEGER_MAX) {
    return SEG_RANGE("Integer out of immediate range.");
  }

  i->header = INTEGER_FLAG | ((uint64_t) value & ~INTEGER_MASK);

  *out = i;
  return SEG_OK;
}

seg_err seg_integer_value(seg_object *object, int64_t *out)
{
  struct imm_integer {
    unsigned top :2;
    int64_t body :62;
  } s;

  if (!IS_INTEGER(object)) {
    return SEG_TYPE("Object was not an integer");
  }

  *out = s.body = object->header;

  return SEG_OK;
}

// SEG_STRINGLIKE //////////////////////////////////////////////////////////////////////////////////

seg_err seg_string(char *str, uint64_t length, seg_object **out)
{
  return SEG_NOTYET("seg_string");
}

seg_err seg_cstring(const char *str, seg_object **out)
{
  return SEG_NOTYET("seg_cstring");
}

seg_err seg_symbol(char *str, uint64_t length, seg_object **out)
{
  return SEG_NOTYET("seg_symbol");
}

seg_err seg_string_name(seg_object *stringlike, char **out)
{
  return SEG_NOTYET("seg_string_name");
}

seg_err seg_string_length(seg_object *stringlike, uint64_t *out)
{
  return SEG_NOTYET("seg_string_length");
}


// SEG_SLOTTED /////////////////////////////////////////////////////////////////////////////////////

seg_err seg_slotted(seg_object *class, seg_object **out)
{
  return SEG_NOTYET("seg_slotted");
}

seg_err seg_slotted_capacity(seg_object *instance, uint64_t *out)
{
  return SEG_NOTYET("seg_slotted_capacity");
}

seg_err seg_slotted_length(seg_object *instance, uint64_t *out)
{
  return SEG_NOTYET("seg_slotted_length");
}

seg_err seg_slotted_setlength(seg_object *instance, uint64_t length)
{
  return SEG_NOTYET("seg_slotted_setlength");
}

seg_err seg_slotted_grow(seg_object *instance, uint64_t capacity)
{
  return SEG_NOTYET("seg_slotted_grow");
}

seg_err seg_ivar_at(seg_object *o, uint64_t index, seg_object **out)
{
  return SEG_NOTYET("seg_ivar_at");
}

seg_err seg_ivar_atput(seg_object *o, uint64_t index, seg_object *v)
{
  return SEG_NOTYET("seg_ivar_atput");
}

// BOOTSTRAPPING ///////////////////////////////////////////////////////////////////////////////////

seg_err _seg_bootstrap_runtime(seg_runtime *runtime, seg_bootstrap_objects *bootstrap)
{
  return SEG_OK;
}
