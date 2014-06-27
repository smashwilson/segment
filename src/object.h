#ifndef OBJECT_H
#define OBJECT_H

#include <stdbool.h>
#include <stdint.h>

struct seg_object;
typedef struct seg_object seg_object;

/*
 * Allocate a new integer object.
 */
seg_object *seg_integer(uint64_t value);

/*
 * Access the value of an integer object.
 */
uint64_t seg_integer_value(seg_object *object);

/*
 * Access an object's class.
 */
seg_object *seg_class(seg_object *object);

/*
 * Return true if `object` is a Class instance.
 */
bool seg_is_class(seg_object *object);

#endif
