#include "compiler/compiler.h"

#include "runtime/bytecode.h"
#include "model/module.h"

typedef struct {
  seg_object *entry_block;

  uint32_t literal_count;
  size_t literal_capacity;
  seg_object *literal_pool;
} seg_module_builder;

seg_err seg_compile(seg_runtime *r, seg_block_node *ast, seg_object *module)
{
  return SEG_NOTYET("seg_compile");
}
