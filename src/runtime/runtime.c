#include <stdlib.h>

#include "runtime/runtime.h"
#include "model/object.h"

struct seg_runtime {
  seg_symboltable *symboltable;
  seg_bootstrap_objects bootstrap;
};

seg_runtime *seg_new_runtime()
{
  seg_runtime *r = malloc(sizeof(seg_runtime));

  /* Initialize the symbol table. */
  r->symboltable = seg_new_symboltable();

  /* Create bootstrap objects. */
  _seg_bootstrap_runtime(r);

  return r;
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
