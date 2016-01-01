#ifndef OPTION_H
#define OPTION_H

#include "model/errors.h"
#include "model/object.h"

/*
 * Construct an Option containing Some existing instance.
 */
seg_err seg_some(seg_runtime *r, seg_object content, seg_object *out);

/*
 * Access the singleton None instance for this runtime.
 */
seg_err seg_none(seg_runtime *r, seg_object *out);

#endif /* end of include guard: OPTION_H */
