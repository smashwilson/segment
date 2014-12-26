#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <stdbool.h>

#include "errors.h"

struct seg_object_common;
typedef struct seg_object_common seg_object_common;

/*
 * Either a pointer to a segment object, or an immediate value.
 *
 * Structure exposed by necessity of avoiding unnecessary heap allocations. Use the construction
 * functions in this file to acquire seg_objects rather than mucking about within them yourself.
 */
typedef union {
  /* Pointer to a heap-allocated seg_object_xyz struct. */
  seg_object_common *pointer;

  /* Immediate value storage. */
  struct {
    /*
    * Because malloc() aligns the memory it allocates (on 64-bit systems, generally on four-byte
    * boundaries), it will never return an odd pointer. This means that we can use the least
    * significant bit as a flag for immediates.
    */
    unsigned immediate: 1;

    /* Enum constant from seg_imm_kinds. */
    unsigned kind: 4;

    /* Number of bytes used by a string or a symbol immediate. */
    unsigned length: 3;

    /* Body of the immediate, interpreted depending on the kind flag. */
    long body: 56;
  } bits;
} seg_object;

/*
 * Storage mechanisms that may be used for a specific instance.
 */
typedef enum {
  SEG_STORAGE_IMMEDIATE = 0,
  SEG_STORAGE_EMPTY,
  SEG_STORAGE_STRINGLIKE,
  SEG_STORAGE_SLOTTED
} seg_storage;

/* Macro to determine whether or not a given seg_object is an immediate or not. */
#define SEG_IS_IMMEDIATE(obj) ((obj).bits.immediate)

/* Macro to convert a seg_object to a pointer. Useful for storage in hashtables. */
#define SEG_TOPOINTER(obj) ((obj).pointer)

/* Macro to convert a void* to a seg_object. Useful for extraction from hashtables. */
#define SEG_FROMPOINTER(p) { .pointer = (seg_object_common*) p }

/* Macro to determine if two seg_objects represent the same instance. */
#define SEG_SAME(a, b) ((a).pointer == (b).pointer)

/* A seg_object that represents an absent value for C APIs. Segment APIs should use None. */
extern const seg_object SEG_NULL;

/* Forward declaration of seg_runtime for the bootstrap function. */
struct seg_runtime;
typedef struct seg_runtime seg_runtime;

/*
 * Access the class of any instance.
 */
seg_object seg_object_class(seg_runtime *r, seg_object object);

/*
 * Return true if the two objects represent the same instance, or false if they don't. Use SEG_SAME
 * in C code.
 */
bool seg_object_same(seg_object a, seg_object b);

/*
 * Determine the underlying storage mechanism used by a specific instance.
 *
 * SEG_INVAL: o is an invalid object.
 */
seg_err seg_object_storage(seg_object o, seg_storage *out);

/*
 * Construct a seg_object from an arbitrary pointer, which must be a seg_object_common*.
 */
seg_object seg_object_frompointer(void *p);

/*
 * Allocate a new integer object.
 *
 * SEG_RANGE: If value uses more than 56 bits.
 */
seg_err seg_integer(seg_runtime *r, int64_t value, seg_object *out);

/*
 * The maximum value that can legally be stored within an immediate integer.
 */
#define SEG_INTEGER_MAX ((int64_t) 0x00ffffffffffffff)

/*
 * The minimum value that can legally be stored within an immediate integer.
 */
#define SEG_INTEGER_MIN ((int64_t) 0xffe0000000000000)

/*
 * Access the value of an immediate integer.
 *
 * SEG_TYPE: If object is not an immediate integer.
 */
seg_err seg_integer_value(seg_object object, int64_t *out);

/*
 * Allocate a new string object. If it's seven bytes or less in length, return an immediate string
 * instead.
 *
 * SEG_NOMEM: If the allocation attempt fails.
 */
seg_err seg_string(seg_runtime *r, const char *str, uint64_t length, seg_object *out);

/*
 * The maximum number of bytes that can be stored in an immediate-value String or Symbol.
 */
#define SEG_STR_IMMLEN 7

/*
 * Determine pre-instantiation whether or not a stringlike object will be an immediate or not.
 */
#define SEG_STR_WILLBEIMM(len) ((len) <= SEG_STR_IMMLEN)

/*
 * Convenience constructor for creating seg_object Strings out of literal, C-style strings. If it's
 * seven bytes or less in length, return an immediate string instead.
 *
 * SEG_NOMEM: If the allocation attempt fails.
 */
seg_err seg_cstring(seg_runtime *r, const char *str, seg_object *out);

/*
 * Allocate a new symbol object. If it's seven bytes or less in length, return an immediate symbol
 * instead. Generally, seg_symboltable_intern() should be used instead to register the new symbol
 * in the symbol table.
 *
 * SEG_NOMEM: If the allocation attempt fails.
 */
seg_err seg_symbol(seg_runtime *r, const char *str, uint64_t length, seg_object *out);

/*
 * Access a symbol or string's contents and length.
 *
 * SEG_TYPE: If stringlike is not a string or symbol.
 */
seg_err seg_stringlike_contents(seg_object *stringlike, char **out, uint64_t *length);

/*
 * Allocate a new slotted instance from a class.
 *
 * SEG_TYPE: If class is not actually a class object.
 * SEG_NOMEM: If the instance allocation fails.
 */
seg_err seg_slotted(seg_runtime *r, seg_object klass, seg_object *out);

/*
 * Return the currently allocated slot length of a class.
 *
 * SEG_TYPE: If instance is not a slotted object.
 */
seg_err seg_slotted_length(seg_object instance, uint64_t *out);

/*
 * Expand the allocated length of a slotted instance. If the instance already has at least the
 * requested length, do nothing.
 *
 * SEG_TYPE: If instance is not a slotted object.
 * SEG_NOMEM: If the allocation fails.
 */
seg_err seg_slotted_grow(seg_object instance, uint64_t length);

/*
 * Access a slot within a slotted object at a specific index.
 *
 * SEG_TYPE: If slotted is not actually a slotted object.
 * SEG_RANGE: If index is outside of the allocated object length.
 */
seg_err seg_slot_at(seg_object slotted, uint64_t index, seg_object *out);

/*
 * Set a slot within a slotted object at a specific index.
 *
 * SEG_TYPE: If slotted is not actually a slotted object.
 * SEG_RANGE: If the index is beyond the object's current ivar capacity.
 */
seg_err seg_slot_atput(seg_object slotted, uint64_t index, seg_object *out);

#define SEG_NO_IVAR UINT64_MAX

/* Forward declared for _seg_bootstrap_runtime. */
struct seg_bootstrap_objects;
typedef struct seg_bootstrap_objects seg_bootstrap_objects;

/*
 * Populate a runtime with an initial set of basic classes and objects.
 */
seg_err _seg_bootstrap_runtime(seg_runtime *runtime, seg_bootstrap_objects *bootstrap);

#endif
