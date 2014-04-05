#ifndef LEXER_H
#define LEXER_H

#include <sys/types.h>

#include "ast.h"
#include "segment.h"

/**
 * Parse a coherent chunk of segment code and return its AST.
 */
seg_statementlist_node *seg_parse(char *content, off_t length, seg_options *opts);

#endif
