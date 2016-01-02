#ifndef COMPILER_H
#define COMPILER_H

#include "model/object.h"
#include "model/errors.h"
#include "compiler/ast.h"
#include "runtime/runtime.h"

/*
 * Compile a parsed and optimized abstract syntax tree into an existing module, populating it with
 * a sequence of literals discovered from this source file. Most notably, at least one of these
 * literals will be a Block object containing bytecode sequences that the interpreter can execute.
 */
seg_err seg_compile(seg_runtime *r, seg_block_node *ast, seg_object *module);

#endif /* end of include guard: COMPILER_H */
