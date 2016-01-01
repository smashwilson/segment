#ifndef MODULE_H
#define MODULE_H

#include "runtime/runtime.h"
#include "runtime/symboltable.h"
#include "model/errors.h"

seg_err seg_module(seg_runtime *r, const char *name, seg_object *out);

#endif /* end of include guard: MODULE_H */
