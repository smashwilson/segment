#include <stdlib.h>

#include "object.h"
#include "symboltable.h"

struct seg_object {
  seg_object *class;
};

typedef struct {
  seg_object object;
  uint64_t value;
} seg_integer_object;

typedef struct {
  seg_object object;
  uint64_t length;
  const char *value;
} seg_string_object;

typedef struct {
  seg_object object;
  seg_symbol *symbol;
} seg_symbol_object;

seg_object *seg_integer(uint64_t value)
{
  seg_integer_object *allocated = malloc(sizeof(seg_integer_object));
  if (allocated == NULL) {
    return NULL;
  }

  // TODO assign the class here.
  allocated->object.class = NULL;

  // TODO register with garbage collection and so forth.
  allocated->value = value;

  return (seg_object *) allocated;
}

uint64_t seg_integer_value(seg_object *object)
{
  // TODO test the object's class.

  seg_integer_object *casted = (seg_integer_object*) object;

  return casted->value;
}

seg_object *seg_class(seg_object *object)
{
  return object->class;
}

/*
 * Return true if `object` is a class.
 */
bool seg_isclass(seg_object *object)
{
  return object->class == object;
}
