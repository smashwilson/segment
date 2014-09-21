#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include <stdint.h>

#include "errors.h"

struct seg_object;
typedef struct seg_object seg_object;

/* Forward declaration of seg_runtime for the bootstrap function. */
struct seg_runtime;
typedef struct seg_runtime seg_runtime;

/*
 * Access the class of any instance.
 */
seg_object *seg_class(seg_object *object, seg_runtime *r);

/*
 * Allocate a new integer object.
 *
 * SEG_NOMEM: If the allocation attempt fails.
 * SEG_RANGE: If value uses more than 62 bits.
 */
seg_err seg_integer(int64_t value, seg_object **out);

/*
 * The maximum value that can legally be stored within an immediate integer.
 */
#define SEG_INTEGER_MAX ((int64_t) 0x1fffffffffffffff)

/*
 * The minimum value that can legally be stored within an immediate integer.
 */
#define SEG_INTEGER_MIN ((int64_t) 0xe000000000000000)

/*
 * Access the value of an integer object.
 *
 * SEG_TYPE: If object is not an integer literal.
 */
seg_err seg_integer_value(seg_object *object, int64_t *out);

/*
 * Allocate a new string object.
 *
 * SEG_NOMEM: If the allocation attempt fails.
 */
seg_err seg_string(char *str, uint64_t length, seg_object **out);

/*
 * Convenience constructor for creating seg_object Strings out of literal, C-style strings.
 *
 * SEG_NOMEM: If the allocation attempt fails.
 */
seg_err seg_cstring(const char *str, seg_object **out);

/*
 * Allocate a new symbol object.
 *
 * SEG_NOMEM: If the allocation attempt fails.
 */
seg_err seg_symbol(char *str, uint64_t length, seg_object **out);

/*
 * Access a symbol or string's contents and length.
 *
 * SEG_TYPE: If stringlike is not a string or symbol.
 */
seg_err seg_string_contents(seg_object *stringlike, char **out, uint64_t *length);

/*
 * Allocate a new slotted instance from a class.
 *
 * SEG_TYPE: If class is not actually a class object.
 * SEG_NOMEM: If the instance allocation fails.
 */
seg_err seg_slotted(seg_object *class, seg_object **out);

/*
 * Return the current slot capacity of a class.
 *
 * SEG_TYPE: If instance is not a slotted object.
 */
seg_err seg_slotted_capacity(seg_object *instance, uint64_t *out);

/*
 * Return the current used slot length of a class.
 *
 * SEG_TYPE: If instance is not a slotted object.
 */
seg_err seg_slotted_length(seg_object *instance, uint64_t *out);

/*
 * Expand the allocated length of a slotted instance.
 *
 * SEG_TYPE: If instance is not a slotted object.
 * SEG_RANGE: If the length is greater than the instance's capacity.
 */
seg_err seg_slotted_setlength(seg_object *instance, uint64_t length);

/*
 * Attempt to expand the capacity of an existing slotted instance.
 *
 * SEG_TYPE: If instance is not a slotted object.
 * SEG_NOMEM: If the allocation fails.
 * SEG_RANGE: If the requested capacity can't fit in 63 bits.
 */
seg_err seg_slotted_grow(seg_object *instance, uint64_t capacity);

/*
 * Access an instance variable within a slotted object at a specific index.
 *
 * SEG_RANGE: If index is outside the allocated object length.
 */
seg_err seg_ivar_at(seg_object *o, uint64_t index, seg_object **out);

/*
 * Set an instance variable within a slotted object at a specific index.
 *
 * SEG_RANGE: If the index is beyond the object's current ivar capacity.
 */
seg_err seg_ivar_atput(seg_object *o, uint64_t index, seg_object *v);

#define SEG_NO_IVAR UINT64_MAX

/* Forward declared for _seg_bootstrap_runtime. */
struct seg_bootstrap_objects;
typedef struct seg_bootstrap_objects seg_bootstrap_objects;

/*
 * Populate a runtime with an initial set of basic classes and objects.
 */
seg_err _seg_bootstrap_runtime(seg_runtime *runtime, seg_bootstrap_objects *bootstrap);

#endif
