#ifndef PARSE_HELPERS
#define PARSE_HELPERS

#include "ast.h"
#include "token.h"

/*
 * Allocate a new seg_expr_node to model a binary operator application. `op` token will be destroyed
 * after use.
 */
seg_expr_node *seg_parse_binop(seg_expr_node *lhs, seg_token *op, seg_expr_node *rhs);

/*
 * Initialize a new seg_arg_list entry. `keyword` may be left NULL. If provided, `keyword` token
 * will be destroyed after use.
 */
seg_arg_list *seg_parse_arg(seg_expr_node *value, seg_token *keyword);

#endif
