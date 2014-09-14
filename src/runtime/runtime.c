#include <stdlib.h>

#include "runtime/runtime.h"
#include "model/object.h"

struct seg_runtime {
  seg_symboltable *symboltable;
  seg_bootstrap_objects bootstrap;
};

seg_err seg_new_runtime(seg_runtime **out)
{
  seg_runtime *r = malloc(sizeof(seg_runtime));
  if (r == NULL) {
    return SEG_NOMEM;
  }

  /* Initialize the symbol table. */
  seg_err err = seg_new_symboltable(&r->symboltable);
  if (err != SEG_OK) {
    return err;
  }

  /* Create bootstrap objects. */
  err = _seg_bootstrap_runtime(r, &r->bootstrap);
  if (err != SEG_OK) {
    return err;
  }

  out = &r;
  return SEG_OK;
}

seg_symboltable *seg_runtime_symboltable(seg_runtime *runtime)
{
  return runtime->symboltable;
}

const seg_bootstrap_objects const *seg_runtime_bootstraps(seg_runtime *runtime)
{
  return &(runtime->bootstrap);
}

void seg_delete_runtime(seg_runtime *runtime)
{
  seg_delete_symboltable(runtime->symboltable);
  free(runtime);
}
