#ifndef PARSE_HELPERS
#define PARSE_HELPERS

#include "ast.h"
#include "token.h"

/*
 * Allocate a new seg_expr_node to model a binary operator application.
 */
seg_expr_node *parse_binop(seg_expr_node *lhs, seg_token *tok, seg_expr_node *rhs);

#endif
