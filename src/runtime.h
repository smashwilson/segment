#ifndef RUNTIME_H
#define RUNTIME_H

#include "symboltable.h"

/*
 * Global interpreter state.
 */
struct seg_runtime;
typedef struct seg_runtime seg_runtime;

/*
 * Initialize the runtime, bootstrapping it with initial objects such as the Class class.
 */
seg_runtime *seg_new_runtime();

/*
 * Access the global symbol table within a given runtime.
 */
seg_symboltable *seg_runtime_symboltable(seg_runtime *runtime);

/*
 * Dispose of a runtime acquired from seg_new_runtime().
 */
void seg_delete_runtime(seg_runtime *runtime);

#endif
