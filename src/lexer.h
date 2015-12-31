#ifndef LEXER_H
#define LEXER_H

#include <sys/types.h>

#include "ast.h"
#include "options.h"
#include "runtime/symboltable.h"

typedef struct {
  seg_block_node *ast;
  seg_symboltable *symboltable;
} seg_program;

/**
 * Parse a coherent chunk of segment code and return its AST.
 */
seg_program *seg_parse(seg_runtime *r, char *content, off_t length, seg_options *opts);

/* The initial size of the Ragel stack. */
#define RAGEL_INIT_STACK_SIZE 5

/* The amount by which the Ragel stack will be increased or decreased. */
#define RAGEL_STACK_INCR 10

#endif
