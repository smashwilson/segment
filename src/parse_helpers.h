#ifndef PARSE_HELPERS
#define PARSE_HELPERS

#include "ast.h"
#include "token.h"

/*
 * Append a new statement to a statement list. If `maybe` is NULL, return the original list
 * unmodified.
 */
seg_statementlist_node *seg_append_statement(seg_statementlist_node *list, seg_expr_node *maybe);

/*
 * Allocate a new seg_expr_node to model a binary operator application. `op` token will be destroyed
 * after use.
 */
seg_expr_node *seg_parse_binop(seg_expr_node *lhs, seg_token *op, seg_expr_node *rhs);

/*
 * Allocate a new seg_expr_node to model an arbitrary method invocation. `selector` token will be
 * destroyed after use. If `trim` is specified, a trailing `(` will be removed from the selector
 * before parsing.
 */
seg_expr_node *seg_parse_methodcall(
  seg_expr_node *receiver,
  seg_token *selector,
  int trim,
  seg_arg_list *args
);

/*
 * Initialize a new seg_arg_list entry. `keyword` may be left NULL. If provided, `keyword` token
 * will be destroyed after use.
 */
seg_arg_list *seg_parse_arg(seg_expr_node *value, seg_token *keyword);

/*
 * Return a seg_expr_node that represents an implicit `self` reference.
 */
seg_expr_node *seg_implicit_self();

/*
 * Reverse an argument list.
 */
seg_arg_list *seg_reverse_args(seg_arg_list *original);

/*
 * Reverse a parameter list.
 */
seg_parameter_list *seg_reverse_params(seg_parameter_list *original);

#endif
