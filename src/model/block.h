#ifndef BLOCK_H
#define BLOCK_H

#include "runtime/runtime.h"
#include "runtime/bytecode.h"
#include "model/errors.h"
#include "model/object.h"

typedef enum {
  SEG_BLOCK_SLOT_NAME = 0,
  SEG_BLOCK_SLOT_VERSION,
  SEG_BLOCK_SLOT_LITERAL_POOL,
  SEG_BLOCK_SLOT_ENTRY_BLOCK,
  SEG_BLOCK_SLOTCOUNT
} seg_block_slots;

/*
 * Construct a Block object containing a compiled bytecode sequence and an optional parent scope.
 */
seg_err seg_block(
  seg_runtime *r,
  seg_instruction *bytecode,
  uint32_t bytecode_length,
  seg_object parent_scope,
  seg_object *out
);

#endif /* end of include guard: BLOCK_H */
