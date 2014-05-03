#ifndef PARSE_HELPERS
#define PARSE_HELPERS

#include <stddef.h>

#include "ast.h"
#include "token.h"
#include "symboltable.h"

/* Methods that have special significance. */

#define SEG_METHOD_STRINGAPPEND "<<"
#define SEG_METHOD_STRINGCONV "as_string"
#define SEG_METHOD_STRINGINTERN "as_symbol"

struct seg_parser_context;
typedef struct seg_parser_context *seg_parser_contextp;

typedef struct {
  seg_block_node *root;
  seg_symboltablep symboltable;
  seg_parser_contextp context;
} seg_parser_state;

/*
 * Push a new `seg_parser_context` onto the stack each time a new block is opened. Notice that the
 * parser context stack should initially be empty (that is, `NULL`).
 */
void seg_parser_pushcontext(seg_parser_state *state, seg_block_node *block);

/*
 * Pop the top `seg_parser_context` from the context stack.
 */
void seg_parser_popcontext(seg_parser_state *state);

/*
 * Add a parameter to the current block.
 */
void seg_parser_addparam(seg_parser_state *state, seg_parameter_list *param);

/*
 * Return true iff `identifier` is used as a block argument in the current or a parent block.
 */
int seg_parser_isarg(seg_parser_state *state, const char *identifier, size_t length);

/*
 * Append a new statement to a statement list. If `maybe` is NULL, return the original list
 * unmodified.
 */
seg_block_node *seg_append_expr(seg_block_node *list, seg_expr_node *maybe);

/*
 * Allocate a new seg_expr_node to model a binary operator application. `op` token will be destroyed
 * after use.
 */
seg_expr_node *seg_parse_binop(
  seg_parser_state *state,
  seg_expr_node *lhs,
  seg_token *op,
  seg_expr_node *rhs
);

/*
 * Allocate a new seg_expr_node to model an arbitrary method invocation. `selector` token will be
 * destroyed after use. If `trim` is specified, a trailing `(` will be removed from the selector
 * before parsing.
 */
seg_expr_node *seg_parse_methodcall(
  seg_parser_state *state,
  seg_expr_node *receiver,
  seg_token *selector,
  int trim,
  seg_arg_list *args
);

/*
 * Allocate a new seg_expr_node to model an implicit method call. `selector` should be a literal
 * string.
 */
seg_expr_node *seg_implicit_methodcall(
  seg_parser_state *state,
  seg_expr_node *receiver,
  const char *selector
);

/*
 * Finish an interpolation expression. `middle` is the actual SEG_METHOD_STRINGAPPEND invocation,
 * so it will actually be returned, with `stem` as its receiver and `end_token` wrapped in a String
 * expression as a final argument.
 *
 * `end_token` will be destroyed by this call.
 */
seg_expr_node *seg_parse_interpolation(
  seg_parser_state *state,
  seg_expr_node *stem,
  seg_expr_node *middle,
  seg_token *end_token
);

/*
 * Initialize a new seg_arg_list entry. `keyword` may be left NULL. If provided, `keyword` token
 * will be destroyed after use.
 */
seg_arg_list *seg_parse_arg(seg_parser_state *state, seg_expr_node *value, seg_token *keyword);

/*
 * Return a seg_expr_node that represents an implicit `self` reference.
 */
seg_expr_node *seg_implicit_self(seg_parser_state *state);

/*
 * Reverse an argument list.
 */
seg_arg_list *seg_reverse_args(seg_arg_list *original);

/*
 * Reverse a parameter list.
 */
seg_parameter_list *seg_reverse_params(seg_parameter_list *original);

#endif
