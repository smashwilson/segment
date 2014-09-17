#ifndef RUNTIME_H
#define RUNTIME_H

#include "errors.h"
#include "runtime/symboltable.h"
#include "model/object.h"

/*
 * Global interpreter state.
 */
struct seg_runtime;
typedef struct seg_runtime seg_runtime;

/*
 * Special objects instantiated runtime initialization that used often enough by the interpreter
 * to justify special access.
 */
struct seg_bootstrap_objects {
  // Notable Class instances.
  seg_object *class_class;
  seg_object *integer_class;
  seg_object *string_class;
  seg_object *symbol_class;
  seg_object *array_class;
  seg_object *block_class;

  // Useful singletons.
  seg_object *empty_instance;
  seg_object *true_instance;
  seg_object *false_instance;
};
typedef struct seg_bootstrap_objects seg_bootstrap_objects;

/*
 * Initialize the runtime, bootstrapping it with initial objects such as the Class class.
 */
seg_err seg_new_runtime(seg_runtime **out);

/*
 * Access the global symbol table within a given runtime.
 */
seg_symboltable *seg_runtime_symboltable(seg_runtime *runtime);

/*
 * Access the read-only bootstrap objects.
 */
const seg_bootstrap_objects *seg_runtime_bootstraps(seg_runtime *runtime);

/*
 * Dispose of a runtime acquired from seg_new_runtime().
 */
void seg_delete_runtime(seg_runtime *runtime);

#endif
