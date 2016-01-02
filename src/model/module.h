#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

#include "runtime/runtime.h"
#include "model/errors.h"
#include "model/object.h"

typedef enum {
  SEG_MODULE_SLOT_NAME = 0,
  SEG_MODULE_SLOT_VERSION,
  SEG_MODULE_ENTRY_BLOCK,
  SEG_MODULE_SLOT_LITERAL_POOL,
  SEG_MODULE_SLOT_EXPORT_CONTEXT,
  SEG_MODULE_SLOTCOUNT
} seg_module_slots;

/*
 * Construct a named Module instance with an empty literal pool and no exported method bindings.
 */
seg_err seg_module(seg_runtime *r, const char *name, const char *version, seg_object *out);

/*
 * Access the entry block of this module.
 */
seg_err seg_module_entry_block(seg_object module, seg_object *out);

/*
 * Populate the entry block of this module.
 */
seg_err seg_module_set_entry_block(seg_object module, seg_object block);

/*
 * Append a sequence of literals to this module.
 */
seg_err seg_module_append(seg_object module, seg_object *literals, uint32_t literal_count);

#endif /* end of include guard: MODULE_H */
