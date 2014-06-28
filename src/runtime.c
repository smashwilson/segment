#include <stdlib.h>

#include "runtime.h"

struct seg_runtime {
  seg_symboltable *symboltable;
};

seg_runtime *seg_new_runtime()
{
  seg_runtime *r = malloc(sizeof(seg_runtime));

  r->symboltable = seg_new_symboltable();

  return r;
}

seg_symboltable *seg_runtime_symboltable(seg_runtime *runtime)
{
  return runtime->symboltable;
}

void seg_delete_runtime(seg_runtime *runtime)
{
  seg_delete_symboltable(runtime->symboltable);
  free(runtime);
}
